import os
import sys
import struct

# node cache to enforce 3 node limit
node_cache = {}
cache_order = [] #track order for lru algo

def get_cached_node(file, block_id):
    # check if node is already in cache and move to end (recently used)
    if block_id in node_cache:
        cache_order.remove(block_id)
        cache_order.append(block_id)
        return node_cache[block_id]
    
    node = read_node_at_block(file, block_id)
    
    # if cache is full, evict least recently used
    if len(node_cache) >= 3:
        lru_block_id = cache_order.pop(0)
        del node_cache[lru_block_id]
    
    #add node
    node_cache[block_id] = node
    cache_order.append(block_id)
    
    return node

def clear_cache():
    global node_cache, cache_order
    node_cache = {}
    cache_order = []

def create(filename):
    # check if file already exists
    if os.path.exists(filename):
        print("Error: File already exists")
        return False

    try:
        with open(filename, "wb") as file:
            # write header: magic number, root id (0 for empty tree), next block id (1)
            MAGIC_NUMBER = b"4348PRJ3"
            root_block_id = (0).to_bytes(8, "big")
            next_block_id = (1).to_bytes(8, "big")
            unused_space = bytes(512 - 24)
            file.write(MAGIC_NUMBER + root_block_id + next_block_id + unused_space)
        return True
    except Exception as e:
        print(f"Error creating file: {e}")
        return False

def insert(filename, key, value):
    try:
        clear_cache()  
        with open(filename, "rb+") as file:
            # read header to get root and next block ids
            file.seek(0)
            header = file.read(24)
            magic_number, root_block_id, next_block_id = struct.unpack(">8sQQ", header)

            # case 1: empty tree, create first node as root
            if root_block_id == 0:
                root_block_id = next_block_id
                next_block_id += 1

                root_node = create_btree_node(root_block_id, 0, [(key, value)])
                file.seek(root_block_id * 512)
                file.write(root_node)

                # update header with new root
                file.seek(8)
                file.write(struct.pack(">QQ", root_block_id, next_block_id))
                return True

            root = get_cached_node(file, root_block_id)
            
            # case 2: root is full, need to split
            if root["num_keys"] == 19:
                new_root_block_id = next_block_id
                next_block_id += 1
                
                # split the old root
                next_block_id, middle_key, middle_value, new_node_block_id = split_node(
                    file, root_block_id, next_block_id, root_block_id
                )
                
                # create new root with middle key
                new_root_keys = [middle_key] + [0] * 18
                new_root_values = [middle_value] + [0] * 18
                new_root_children = [root_block_id, new_node_block_id] + [0] * 18 # old root and new node as children
                
                node_format = ">QQQ" + "Q" * 19 + "Q" * 19 + "Q" * 20
                new_root_node = struct.pack(node_format,
                    new_root_block_id, 0, 1,
                    *new_root_keys, *new_root_values, *new_root_children)
                
                file.seek(new_root_block_id * 512)
                file.write(new_root_node)
                
                # update parent ids of children
                update_parent_id(file, root_block_id, new_root_block_id)
                update_parent_id(file, new_node_block_id, new_root_block_id)
                
                # update header with new root
                file.seek(8)
                file.write(struct.pack(">QQ", new_root_block_id, next_block_id))
                
                root_block_id = new_root_block_id
            
            # insert into non-full root
            next_block_id = insert_non_full(file, root_block_id, key, value, next_block_id)
            
            # update next block id in header
            file.seek(16)
            file.write(struct.pack(">Q", next_block_id))
            
            return True
    except Exception as e:
        print(f"Error during insert: {e}")
        return False
    finally:
        clear_cache() 

def insert_non_full(file, block_id, key, value, next_block_id):
    node = get_cached_node(file, block_id)
    
    # case 1: node is a leaf, insert directly
    if is_leaf(node):
        keys = node["keys"][:node["num_keys"]]
        values = node["values"][:node["num_keys"]]
        
        # find position to insert key
        insert_pos = 0
        while insert_pos < len(keys) and keys[insert_pos] < key:
            insert_pos += 1
        
        # insert key and value at position
        keys.insert(insert_pos, key)
        values.insert(insert_pos, value)
        
        keys_padded = keys + [0] * (19 - len(keys))
        values_padded = values + [0] * (19 - len(values))
        
        # write updated node back to disk
        node_format = ">QQQ" + "Q" * 19 + "Q" * 19 + "Q" * 20
        updated_node = struct.pack(node_format,
            node["block_id"], node["parent_id"], len(keys),
            *keys_padded, *values_padded, *node["children"])
        
        file.seek(block_id * 512)
        file.write(updated_node)
        
        return next_block_id
    else:
        # case 2: internal node, find correct child
        i = 0
        while i < node["num_keys"] and key > node["keys"][i]:
            i += 1
        
        child_id = node["children"][i]
        child = get_cached_node(file, child_id)
        
        # if child is full, split it first
        if child["num_keys"] == 19:
            next_block_id, middle_key, middle_value, new_child_id = split_node(
                file, child_id, next_block_id, block_id
            )
            
            # reload parent after split
            node = get_cached_node(file, block_id)
            
            # insert middle key into parent
            keys = list(node["keys"][:node["num_keys"]])
            values = list(node["values"][:node["num_keys"]])
            children = list(node["children"][:node["num_keys"] + 1])
            
            insert_pos = 0
            while insert_pos < len(keys) and keys[insert_pos] < middle_key:
                insert_pos += 1
            
            keys.insert(insert_pos, middle_key)
            values.insert(insert_pos, middle_value)
            children.insert(insert_pos + 1, new_child_id)
            
            keys_padded = keys + [0] * (19 - len(keys))
            values_padded = values + [0] * (19 - len(values))
            children_padded = children + [0] * (20 - len(children))
            
            # write updated parent back
            node_format = ">QQQ" + "Q" * 19 + "Q" * 19 + "Q" * 20
            updated_node = struct.pack(node_format,
                node["block_id"], node["parent_id"], len(keys),
                *keys_padded, *values_padded, *children_padded)
            
            file.seek(block_id * 512)
            file.write(updated_node)
            
            # adjust child index if key is greater than middle key
            if key > middle_key:
                i += 1
            
            child_id = children[i]
        
        # recursively insert into child
        next_block_id = insert_non_full(file, child_id, key, value, next_block_id)
        
        return next_block_id

def split_node(file, block_id, next_block_id, parent_id):
    node = get_cached_node(file, block_id)
    
    # split at middle index (9)
    middle_index = 9
    middle_key = node["keys"][middle_index]
    middle_value = node["values"][middle_index]
    
    # left node gets keys 0-8
    left_keys = node["keys"][:middle_index] + [0] * (19 - middle_index)
    left_values = node["values"][:middle_index] + [0] * (19 - middle_index)
    left_children = node["children"][:middle_index + 1] + [0] * (20 - middle_index - 1)
    
    # right node gets keys 10-18
    right_keys = node["keys"][middle_index + 1:node["num_keys"]] + [0] * (19 - (node["num_keys"] - middle_index - 1))
    right_values = node["values"][middle_index + 1:node["num_keys"]] + [0] * (19 - (node["num_keys"] - middle_index - 1))
    right_children = node["children"][middle_index + 1:node["num_keys"] + 1] + [0] * (20 - (node["num_keys"] - middle_index))
    
    node_format = ">QQQ" + "Q" * 19 + "Q" * 19 + "Q" * 20
    
    # write left node at original block
    left_node = struct.pack(node_format,
        block_id, parent_id, middle_index,
        *left_keys, *left_values, *left_children)
    
    # write right node at new block
    new_block_id = next_block_id
    next_block_id += 1
    
    right_node = struct.pack(node_format,
        new_block_id, parent_id, node["num_keys"] - middle_index - 1,
        *right_keys, *right_values, *right_children)
    
    file.seek(block_id * 512)
    file.write(left_node)
    
    file.seek(new_block_id * 512)
    file.write(right_node)
    
    # update parent ids of children
    for child_id in left_children:
        if child_id != 0:
            update_parent_id(file, child_id, block_id)
    
    for child_id in right_children:
        if child_id != 0:
            update_parent_id(file, child_id, new_block_id)
    
    return next_block_id, middle_key, middle_value, new_block_id

def read_node_at_block(file, block_id):
    # seek to block position and read 512 bytes
    file.seek(block_id * 512)
    node_data = file.read(512)
    return read_btree_node(node_data)

def read_btree_node(data):
    # unpack node data from binary format
    node_data = data[:488] # skip unused bytes
    node_format = ">QQQ" + "Q" * 19 + "Q" * 19 + "Q" * 20
    unpacked = struct.unpack(node_format, node_data)

    return {
        "block_id": unpacked[0],
        "parent_id": unpacked[1],
        "num_keys": unpacked[2],
        "keys": list(unpacked[3:22]),
        "values": list(unpacked[22:41]),
        "children": list(unpacked[41:61]),
    }

def print_btree(filename):
    try:
        clear_cache()
        with open(filename, "rb") as file:
            file.seek(8)
            root_block_id = int.from_bytes(file.read(8), "big")
            
            if root_block_id == 0:
                return
            
            # collect all key-value pairs using dfs
            key_value_pairs = []
            dfs(file, root_block_id, key_value_pairs)
            
            # print each pair
            for key, value in key_value_pairs:
                print(f"{key},{value}")
    except Exception as e:
        print(f"Error printing: {e}")
    finally:
        clear_cache()

def dfs(file, block_id, key_value_pairs):
    # in-order traversal of b-tree
    if block_id == 0:
        return
    
    node = get_cached_node(file, block_id)
    
    for i in range(node["num_keys"]):
        dfs(file, node["children"][i], key_value_pairs) # visit left child
        key_value_pairs.append((node['keys'][i], node['values'][i])) # visit key
    
    dfs(file, node["children"][node["num_keys"]], key_value_pairs) # visit rightmost child

def extract(filename, output_file):
    if os.path.exists(output_file):
        print("Error: Output file already exists")
        return False

    try:
        clear_cache() 
        with open(filename, "rb") as file:
            file.seek(8)
            root_block_id = int.from_bytes(file.read(8), "big")

            if root_block_id == 0:
                return False

            # collect all pairs using dfs
            key_value_pairs = []
            dfs(file, root_block_id, key_value_pairs)

        # write to csv file
        with open(output_file, "w") as out_file:
            out_file.write("\n".join(f"{key},{value}" for key, value in key_value_pairs))
        return True
    except Exception as e:
        print(f"Error during extract: {e}")
        return False
    finally:
        clear_cache()

def load(filename, csv_file):
    if not os.path.exists(csv_file):
        print(f"Error: CSV file does not exist")
        return False

    try:
        # read csv and insert each pair
        with open(csv_file, "r") as file:
            for line in file:
                if not line.strip():
                    continue

                parts = line.strip().split(',')
                key = int(parts[0])
                value = int(parts[1])
                insert(filename, key, value)
        return True
    except Exception as e:
        print(f"Error during load: {e}")
        return False

def search(filename, key):
    try:
        clear_cache() 
        with open(filename, "rb") as file:
            file.seek(8)
            root_block_id = int.from_bytes(file.read(8), "big")

            if root_block_id == 0:
                return None

            # traverse tree to find key
            current_block_id = root_block_id

            while current_block_id != 0:
                node = get_cached_node(file, current_block_id)

                # check if key is in current node
                for i in range(node["num_keys"]):
                    if node["keys"][i] == key:
                        return node["values"][i]

                # find correct child to descend into
                i = 0
                while i < node["num_keys"] and key > node["keys"][i]:
                    i += 1

                current_block_id = node["children"][i]

            return None
    except Exception as e:
        print(f"Error during search: {e}")
        return None
    finally:
        clear_cache()

def update_parent_id(file, block_id, new_parent_id):
    # update parent id field in node
    file.seek(block_id * 512 + 8) # parent id is at offset 8
    file.write(struct.pack(">Q", new_parent_id))

def is_leaf(node):
    # node is leaf if all children are 0
    return all(child == 0 for child in node["children"])

def create_btree_node(block_id, parent_id, key_value_pairs):
    keys = [0] * 19
    for i, (key, _) in enumerate(key_value_pairs):
        keys[i] = key
    
    values = [0] * 19
    for i, (_, value) in enumerate(key_value_pairs):
        values[i] = value
    
    offset = [0] * 20
    
    # pack node into binary format
    node_format = ">QQQ" + "Q" * 19 + "Q" * 19 + "Q" * 20
    node_data = struct.pack(node_format, 
        block_id, parent_id, len(key_value_pairs), 
        *keys, *values, *offset)
    
    return node_data

def validate_index_file(filename):
    if not os.path.exists(filename):
        print("Error: File does not exist")
        return False
    
    try:
        with open(filename, "rb") as file:
            magic_number = file.read(8)
            if magic_number != b"4348PRJ3":
                print("Error: Invalid index file")
                return False
        return True
    except Exception as e:
        print(f"Error validating file: {e}")
        return False

def main():
    if len(sys.argv) < 3:
        print("Usage: python main.py <command> <index-file> <params>")
        return

    command = sys.argv[1].lower()
    index_file = sys.argv[2]

    if command == "create":
        create(index_file)
    elif command == "insert":
        if len(sys.argv) < 5:
            print("Usage: insert <index-file> <key> <value>")
            return
        if not validate_index_file(index_file):
            return
        key = int(sys.argv[3])
        value = int(sys.argv[4])
        insert(index_file, key, value)
    elif command == "search":
        if len(sys.argv) < 4:
            print("Usage: search <index-file> <key>")
            return
        if not validate_index_file(index_file):
            return
        key = int(sys.argv[3])
        result = search(index_file, key)
        if result is not None:
            print(f"{key},{result}")
        else:
            print("Not found")
    elif command == "load":
        if len(sys.argv) < 4:
            print("Usage: load <index-file> <csv-file>")
            return
        if not validate_index_file(index_file):
            return
        csv_file = sys.argv[3]
        load(index_file, csv_file)
    elif command == "print":
        if not validate_index_file(index_file):
            return
        print_btree(index_file)
    elif command == "extract":
        if len(sys.argv) < 4:
            print("Usage: extract <index-file> <output-file>")
            return
        if not validate_index_file(index_file):
            return
        output_file = sys.argv[3]
        extract(index_file, output_file)

if __name__ == "__main__":
    main()