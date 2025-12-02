12/01/2025 5:54 AM
Goals for this session:

- read through project requirements
  I have begun working on this project. The first thing i need to do is to read through the project pdf so i can get a better understanding of what exactly it is that i need t complete. So from the description i need to write an interactive program that creates and manages index files and each index file will contain a b-tree. There are some commands the user needs to be able to use to manage index files:
- These commands will be command line arguments and i need to handle any errors in user inout
- The index file will represent a B-tree and the implementation should not have more than 3 nodes in memory at a time
- Commands should be lowercase:
  - create:
    - creates a new index file
    - if the file exists already then fail with err message
  - insert
    - inserts a key/value pair into the B-tree
    - if the file does not exist or is not a valid index file then exit wit err
  - search
    - searches for a key in the index
    - if the file does not exist or if the file is not a valid index then exit with an error
    - if key/val pair is found return the pair if not print an error message
  - load
    - loads a csv file into the index file
    - each line in the csv is comma separated key/value pair
    - if any of the files doesnt exist or arent in the right format then exit with an error
    - use the insert command to insert each key/value pair
  - print
    - print every key/value pair in the index to standard output
    - if the file does not exist or if the file is not a valid index then exit with an err
  - extract
    - save every key/value pai in the index as comma separated pairs to th file
    - if any of the files doesnt exist or arent in the right format then exit with an error

so i have been able to read through the requirements for this project and i have a basic idea of what it is i need to implement. I will be using C++ to complete this project because i would like to deepen my understanding of low level languages.

In terms of the index file, it will be divided into blocks of 512 bytes so for example, the first 0x0 - 0x1FF bytes will contain header information, then the next 0x200 - 0x3FF bytes will be for the first B tree and so on
All numbers stored in the file should be stores as 8 byte integers with the big endian order (the computer stores bits with MSB first)
The header information needs to be as follows
Magic number -> ID of block containing root node (0 if the tree is empty) -> ID of the next block to be added to the file -> remaining unused bytes
The B trees should have minumal degree 10, each node should be stored in a single 512 block and contain some header information.
Block ID of the node -> Block ID of parent node (if root then 0) -> Number of key/value pairs in this node -> sequence of 19 64 bit keys -> sequence of 19 64 bit values -> sequence of 20 64 bit offets (child pointers for the node) and each sequence of keys correspond to each other.

I have read through the entire project requirement and now have a basic understanding of what i need to do. I will begin work later. Before the next session, i want to read more about index files and B-trees so i come back better equipped to handle this task
12/1/2025 6:31 AM
Reflecting on this session, i have been able to read through the project requirements, i am not going to say whether it looks easy or not but it looks doable.

12/1/2025 10:41 AM
The goal of this session is to get the basic parts of the projcet working, like the command line arguments and errors for the files. The goal of this sessions is to:
implement at least the insert, create, and search commands. This is just to make to computer read the commands and interprete them as commands, not the actual implementation.
I realized i migh tbe committing a lot so i created a bash script that automatically commits for me
i included a gitignore file so as to prevent unnecessary files from being committed. I will start with the project now.

The first thing i want to do is figure out the byte and big endian situation first so i will implement the big endian function that was provided in the project files

i have added the two functions to reverse bytes, now i just need to test it out

so after trying it out i have realized that my computer using little endian, so i will now perform a check to see if the reverse bytes function works now. I just tried the reverse function and it works so the endianness and the byte format has been squared away i will now focus on the commands.

i have been able to create the insert and create commands, the user is able to type those commands into the cli and expect output although i have not fully implemented them yet. I am running into some trouble figuring out how to write the block ids to the file after i create a file but what will be for another session.

12/01/2025 12:51 PM
I was able to figure out the endianness of my computer and also the byte format, i was aleo able to create the insert and search commands, so two out of 3 planned commands were partially implemented. in the next session i plan to figure out how to write to the index file properly and implement the rest of the commands.

12/02/2025 09:49 AM
I am starting a new session. in this session i plan to get all the commands working partially (the computer an recognize the commands) and i also plan to fix the bug that is not letting me read variable data into the file. I will start by editing my bash script to commit because the commit message doesnt seem to be changing

ok that seems to fix it, it actually did not, i just needed to pass in the msg variable in quotes with a dollar sign in front, silly me. it should be fixed now
okay its fixed, time to work on the project
i will start where i left off by trying to partially implement the create command
i figured out the bug, it was kinda obvious but regardless, its done. now the create is partially implemented so i will move on to the other commands

so i have been able to partially implement all the commands, its time to implement the B tree structure, i know i need a node class to represent a node and also i should keep the 3 node constraint in mind. so the B tree has a couple of extra info to keep in mind about,
The B tree has:
The block id of the node
The block id of the nodes parent(if root then 0)
The number of key/value pairs currently in this node
sequence of 19 64 bit keys
sequence of 19 64 bit values
sequence of 20 bit offsets (child pointers for the node)
so the class structure will look like this:
class Node{
uint64_t blockID
uint64_t parent_ID
uint64_t currentNodes
uint64_t keys[19]
uint64_t values[19]
uint64_t offsets[20]

}

this is a rough sketch of what i need for the node class, more research will be done to know the exact structure
i was off by one on the array sizes but everything else tracks, i will implement the class now
i have added the node class, the next thing to do is to fully implement all the functions, since the create function has basically been fully implemented already, i will begin with the insert function.

i have started on the insert function however i will ending this session here because i need a break

12/02/2025 11:17PM
I have been able to partially implement most of the functions, the create function has been fully implemented. next session, the goal is to fuly implement the insert function
