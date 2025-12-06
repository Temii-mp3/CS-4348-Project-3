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