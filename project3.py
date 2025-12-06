import os
import sys
import struct

def create(filename):
    if os.path.exists(filename):
        print("Error: File already exists")
        return False

    try:
        with open(filename, "wb") as file:
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
        with open(filename, "rb+") as file:
            file.seek(0)
            header = file.read(24)
            magic_number, root_block_id, next_block_id = struct.unpack(">8sQQ", header)

            if root_block_id == 0:
                root_block_id = next_block_id
                next_block_id += 1

                root_node = create_btree_node(root_block_id, 0, [(key, value)])
                file.seek(root_block_id * 512)
                file.write(root_node)

                file.seek(8)
                file.write(struct.pack(">QQ", root_block_id, next_block_id))
                return True

            root = read_node_at_block(file, root_block_id)
            
            if root["num_keys"] == 19:
                new_root_block_id = next_block_id
                next_block_id += 1
                
                next_block_id, middle_key, middle_value, new_node_block_id = split_node(
                    file, root_block_id, next_block_id, root_block_id
                )
                
                new_root_keys = [middle_key] + [0] * 18
                new_root_values = [middle_value] + [0] * 18
                new_root_children = [root_block_id, new_node_block_id] + [0] * 18
                
                node_format = ">QQQ" + "Q" * 19 + "Q" * 19 + "Q" * 20
                new_root_node = struct.pack(node_format,
                    new_root_block_id, 0, 1,
                    *new_root_keys, *new_root_values, *new_root_children)
                
                file.seek(new_root_block_id * 512)
                file.write(new_root_node)
                
                update_parent_id(file, root_block_id, new_root_block_id)
                update_parent_id(file, new_node_block_id, new_root_block_id)
                
                file.seek(8)
                file.write(struct.pack(">QQ", new_root_block_id, next_block_id))
                
                root_block_id = new_root_block_id
            
            next_block_id = insert_non_full(file, root_block_id, key, value, next_block_id)
            
            file.seek(16)
            file.write(struct.pack(">Q", next_block_id))
            
            return True
    except Exception as e:
        print(f"Error during insert: {e}")
        return False
    
def insert_non_full(file, block_id, key, value, next_block_id):
    node = read_node_at_block(file, block_id)
    
    if is_leaf(node):
        keys = node["keys"][:node["num_keys"]]
        values = node["values"][:node["num_keys"]]
        
        insert_pos = 0
        while insert_pos < len(keys) and keys[insert_pos] < key:
            insert_pos += 1
        
        keys.insert(insert_pos, key)
        values.insert(insert_pos, value)
        
        keys_padded = keys + [0] * (19 - len(keys))
        values_padded = values + [0] * (19 - len(values))
        
        node_format = ">QQQ" + "Q" * 19 + "Q" * 19 + "Q" * 20
        updated_node = struct.pack(node_format,
            node["block_id"], node["parent_id"], len(keys),
            *keys_padded, *values_padded, *node["children"])
        
        file.seek(block_id * 512)
        file.write(updated_node)
        
        return next_block_id
    else:
        i = 0
        while i < node["num_keys"] and key > node["keys"][i]:
            i += 1
        
        child_id = node["children"][i]
        child = read_node_at_block(file, child_id)
        
        if child["num_keys"] == 19:
            next_block_id, middle_key, middle_value, new_child_id = split_node(
                file, child_id, next_block_id, block_id
            )
            	
            node = read_node_at_block(file, block_id)
            
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
            
            node_format = ">QQQ" + "Q" * 19 + "Q" * 19 + "Q" * 20
            updated_node = struct.pack(node_format,
                node["block_id"], node["parent_id"], len(keys),
                *keys_padded, *values_padded, *children_padded)
            
            file.seek(block_id * 512)
            file.write(updated_node)
            
            if key > middle_key:
                i += 1
            
            child_id = children[i]
        
        next_block_id = insert_non_full(file, child_id, key, value, next_block_id)
        
        return next_block_id

def read_node_at_block(file, block_id):
    file.seek(block_id * 512)
    node_data = file.read(512)
    return read_btree_node(node_data)

def read_btree_node(data):
    node_data = data[:488]
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


def update_parent_id(file, block_id, new_parent_id):
    file.seek(block_id * 512 + 8)
    file.write(struct.pack(">Q", new_parent_id))

def is_leaf(node):
    return all(child == 0 for child in node["children"])

def create_btree_node(block_id, parent_id, key_value_pairs):
    keys = [0] * 19
    for i, (key, _) in enumerate(key_value_pairs):
        keys[i] = key
    
    values = [0] * 19
    for i, (_, value) in enumerate(key_value_pairs):
        values[i] = value
    
    offset = [0] * 20
    
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