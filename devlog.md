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

12/02/2025 9:43 PM
The goal of this session is to see how far i can get with implementing the insert function and any other functions, i plan to work on this for ~30 mins so we'll see how far we get

i noticed that i will be reading/writing from/to the header so i will create a struct to hold that information

i have created the header struct, i will not create helper functions that read/write from/to the header so as to prevent code bloat

i have added the header functions, i realized that before being able to insert keys we need to be able to load the node from memory. I will implement both load node and save node functions that enable us to load and save nodes from/to disk. i also need to not forget the 3 nodes in ram rule

I have been able to add 4 node related functions, including loadnode which loads a node from disk and save node which saves a node to disk. I also added a function to create an empty node for situations where the index file does not contain any nodes. Lastly, i added a helper that will check if a node is leaf (just checks if child pointers are all 0s). These functions will help reduce code bloat and make my program more readable but i am tired and have gone past the 30 minuites allocated to this so i will continue tomorrow
12/02/2025 10:44 PM
I was able to add several helper functions that will help me reduce code bloat, i added functions to load and save nodes to disk and i also added a header struct so its easier to maintain header state. Next session involves working on the insert function and testing the insert command.

12/05/2025 7:00 PM
I will now begin work on creating the insert function that will allow me to insert keys into the B tree. So i know in order to insert we need informatiojn from the header so we know where the current root node is and also the ID of the next available block. There are 2 cases i need to keep in mind, case 1 is if the tree is empty when i just create an empty node and save the node to disk then update the header. the second case has 2 subcases, if the tree exists, but the root is full and i need to split, then ill split the root into 2 and make the old root a child of the new root, then i;; split the old root, updae the header, insert the key into the new root, and update the header again.
The next case is if the root is not full, in that case ill insert the key into the node normally.

okay so i got the basic insert logic done and im testing it now with ./proj insert 20 4 but the hex dump is showing garbage for the value. wait let me check the code... oh its because im casting the pointer directly to uint64_t instead of converting the string. yeah that makes sense, fixed it with the convertToUint64 function

alright now the hex dump looks good, values are showing up correctly in big endian. time to test with more values to make sure splitting works

okay so i created a bash script to insert keys 1-30 and the tree split but now search isnt finding anything. let me check the hex dump. the structure looks off. block 2 should be the root but the data at that position has the wrong block id.

oh wait i think i see it, after splitChild saves the node, the in-memory copy is stale. so when i use it after the split its got old data. let me add a reload after splitChild... yeah that should fix it

testing search for 15... still says not found. let me look at the hex dump again. the root only has 1 key but it should have 2 after 30 insertions. somethings wrong with how multiple splits are being handled

been staring at this for like 2 hours now and every time i fix something else breaks. the splitting logic is really hard to debug for me ngl...maybe C++ was a bad idea lol. Im going to sleep now BUT the session will still be ongoing.

okay, im awake now and after looking at thing for 2 extra hours im not sure exactly where i messed up and i think with the time constraint that i have its best if i switch to python because itll be easier to understand and debug without syntax getting in the way. I will change the file and start from the beginning with just the commands partially implemented.

12/06/2025 1:33 PM
This session was the longest ive had till date (i did sleep inbetween) i was able to implement the insert function and it works, but it starts getting wonky when splits are involved and i dont want to deal with that headache right now so i will be switching to python. Next session will involve creating the python file and partially implementing the commands, just like what i did before

12/06/2:03 PM
After a quick break, i have switched the files so now i am using python. I have also partially implemented the commands and i am able to read commands from the command line. i also added a validate method for the index file so we know if the index file is valid or not. The next thing to do now is to start by implementing the easier commands first then we go to the hard ones so ill start with create first.

Okay i implemented the create, it is wayy easier to do it in python so this shouldnt take long, i already knew how to implement it in c++ so it was just a matter of looking up syntax rules for python. I actually better if i go head first into the harder functions to implement. So the next function to implement would be the insert function. i will start by implementing a few helper funtions like last time before the actual insert function
