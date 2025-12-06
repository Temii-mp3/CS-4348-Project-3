#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <string.h>
#include <vector>
#include <utility>
#define MAGIC "4348PRJ3"

struct Header
{
    char magic[8];
    uint64_t rootID;
    uint64_t nextBlockID;
};

class Node
{
public:
    uint64_t blockID;
    uint64_t parent_ID;
    uint64_t currentPairs;
    uint64_t keys[19];
    uint64_t values[19];
    uint64_t offsets[20];
};

int isBigEndian();
uint64_t reverseBytes(uint64_t x);
bool isIndexFile(std::string);
bool readHeader(std::string, Header &);
bool writeHeader(std::string, uint64_t, uint64_t);
Node loadNode(std::string, uint64_t);
bool saveNode(std::string, Node &);
Node createEmptyNode(uint64_t, uint64_t, bool);
bool isLeaf(Node &);
int findInsertPosition(Node &, uint64_t);
void splitChild(std::string, Node &, int, uint64_t &);
void insertNonFull(std::string, Node &, uint64_t, uint64_t, uint64_t &);
bool insertKey(std::string, uint64_t, uint64_t);
bool convertToUint64(const char *, uint64_t &);
bool searchKey(std::string, uint64_t, uint64_t &);

Node possibleNodes[3];
uint64_t rootID;
uint64_t nextBlockID;

int main(int argc, char **argv)
{
    if (isBigEndian() == 0)
    {
        rootID = reverseBytes(0);
        nextBlockID = reverseBytes(1);
    }
    else
    {
        rootID = 0;
        nextBlockID = 1;
    }

    std::fstream file;

    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <command> <index-file> <optional-params>" << std::endl;
        return 1;
    }
    std::string command = argv[1];
    std::string indexFile = argv[2];

    for (char &c : command)
    {
        c = tolower(c);
    }

    if (command == "insert")
    {
        uint64_t key;
        uint64_t value;
        if (indexFile.empty())
        {
            std::cerr << "Usage: " << command << " insert <index-file> <key> <value>" << std::endl;
            return 1;
        }
        if (!isIndexFile(indexFile))
        {
            std::cerr << "Invalid file, file must be an index file";
            return 1;
        }
        if (convertToUint64(argv[3], key) && convertToUint64(argv[4], value))
        {
            std::cout << "Inserting...." << std::endl;

            insertKey(indexFile, key, value);
        }
        else
        {
            std::cerr << "Invalid key" << std::endl;
        }
    }
    else if (command == "create")
    {
        if (indexFile.empty())
        {
            std::cerr << "Usage: " << command << " insert <index-file> <key> <value>" << std::endl;
            return 1;
        }
        std::string fileToCreate = indexFile + ".idx";

        if (std::filesystem::exists(fileToCreate))
        {
            std::cerr << "File already exists" << std::endl;
            return 1;
        }

        file.open(fileToCreate, std::ios::out | std::ios::binary);
        char zeros[488] = {0};
        if (file.is_open())
        {
            file.write(MAGIC, 8);
            file.write((char *)&rootID, 8);
            file.write((char *)&nextBlockID, 8);
            file.write(zeros, 488);
            file.close();
            std::cout << "File Created Successfully " << std::endl;
        }
        else
        {
            std::cerr << "Could not create file" << std::endl;
            return 1;
        }
    }
    else if (command == "search")
    {
        uint64_t key;
        uint64_t value;
        if (indexFile.empty())
        {
            std::cerr << "Usage: " << command << " search <index-file> <key>" << std::endl;
            return 1;
        }
        if (!isIndexFile(indexFile))
        {
            std::cerr << "Invalid file, file must be an index file";
            return 1;
        }

        if (convertToUint64(argv[3], key))
        {
            std::cout << "Searching...." << std::endl;

            if (!searchKey(indexFile, key, value))
            {
                std::cout << "Not found" << std::endl;
            }
            else
            {
                std::cout << key << "," << value << std::endl;
            }
        }
    }
    else if (command == "load")
    {
        std::string csvFile = argv[3];
        if (indexFile.empty() || csvFile.empty())
        {
            std::cerr << "Usage: " << command << " load <index-file> <csv-file>" << std::endl;
            return 1;
        }
        if (!isIndexFile(indexFile))
        {
            std::cerr << "Invalid file, file must be an index file";
            return 1;
        }
    }
    else if (command == "print")
    {
        if (indexFile.empty())
        {
            std::cerr << "Usage: " << command << " insert <index-file> <key> <value>" << std::endl;
            return 1;
        }
        if (!isIndexFile(indexFile))
        {
            std::cerr << "Invalid file, file must be an index file";
            return 1;
        }
    }
    else if (command == "extract")
    {
        std::string csvFile = argv[3];
        if (indexFile.empty() || csvFile.empty())
        {
            std::cerr << "Usage: " << command << " load <index-file> <csv-file>" << std::endl;
            return 1;
        }
        if (!isIndexFile(indexFile))
        {
            std::cerr << "Invalid file, file must be an index file";
            return 1;
        }
    }
    else
    {
        std::cerr << "Usage: " << argv[0] << " <command> <index-file> <optional-params>" << std::endl;
        return 1;
    }
    return 0;
}

int isBigEndian()
{
    int num = 1;
    return (*(char *)&num == 0);
}

uint64_t reverseBytes(uint64_t x)
{
    uint8_t dest[sizeof(uint64_t)];
    uint8_t *src = (uint8_t *)&x;
    for (int c = 0; c < sizeof(uint64_t); c++)
    {
        dest[c] = src[sizeof(uint64_t) - c - 1];
    }

    return *(uint64_t *)dest;
}

bool isIndexFile(std::string filename)
{
    std::ifstream infile(filename, std::ios::binary);
    if (!infile)
    {
        std::cerr << "File does not exist, please create it first" << std::endl;
        return false;
    }
    char buffer[8];

    infile.read(buffer, 8);

    if (strncmp(buffer, MAGIC, 8) == 0)
    {
        return true;
    }

    return false;
}

bool readHeader(std::string filename, Header &header)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        return false;
    }

    file.read(header.magic, 8);

    file.read((char *)&header.rootID, 8);
    if (!isBigEndian())
    {
        header.rootID = reverseBytes(header.rootID);
    }

    file.read((char *)&header.nextBlockID, 8);
    if (!isBigEndian())
    {
        header.nextBlockID = reverseBytes(header.nextBlockID);
    }

    file.close();
    return true;
}

bool writeHeader(std::string filename, uint64_t rootID, uint64_t nextBlockID)
{
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file)
    {
        return false;
    }

    uint64_t root = rootID;
    uint64_t next = nextBlockID;
    if (!isBigEndian())
    {
        root = reverseBytes(root);
        next = reverseBytes(next);
    }

    file.seekp(0, std::ios::beg);
    file.write(MAGIC, 8);
    file.write((char *)&root, 8);
    file.write((char *)&next, 8);

    file.close();
    return true;
}

Node loadNode(std::string filename, uint64_t blockID)
{
    Node node;
    std::ifstream file(filename, std::ios::binary);

    file.seekg(blockID * 512, std::ios::beg);

    file.read((char *)&node.blockID, 8);
    if (!isBigEndian())
    {
        node.blockID = reverseBytes(node.blockID);
    }

    file.read((char *)&node.parent_ID, 8);
    if (!isBigEndian())
    {
        node.parent_ID = reverseBytes(node.parent_ID);
    }

    file.read((char *)&node.currentPairs, 8);
    if (!isBigEndian())
    {
        node.currentPairs = reverseBytes(node.currentPairs);
    }

    for (int i = 0; i < 19; i++)
    {
        file.read((char *)&node.keys[i], 8);
        if (!isBigEndian())
        {
            node.keys[i] = reverseBytes(node.keys[i]);
        }
    }

    for (int i = 0; i < 19; i++)
    {
        file.read((char *)&node.values[i], 8);
        if (!isBigEndian())
        {
            node.values[i] = reverseBytes(node.values[i]);
        }
    }

    for (int i = 0; i < 20; i++)
    {
        file.read((char *)&node.offsets[i], 8);
        if (!isBigEndian())
        {
            node.offsets[i] = reverseBytes(node.offsets[i]);
        }
    }

    file.close();
    return node;
}

bool saveNode(std::string filename, Node &node)
{
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file)
    {
        return false;
    }

    file.seekp(node.blockID * 512, std::ios::beg);

    uint64_t blockID = node.blockID;
    if (!isBigEndian())
    {
        blockID = reverseBytes(blockID);
    }
    file.write((char *)&blockID, 8);

    uint64_t parentID = node.parent_ID;
    if (!isBigEndian())
    {
        parentID = reverseBytes(parentID);
    }
    file.write((char *)&parentID, 8);

    uint64_t numKeys = node.currentPairs;
    if (!isBigEndian())
    {
        numKeys = reverseBytes(numKeys);
    }
    file.write((char *)&numKeys, 8);

    for (int i = 0; i < 19; i++)
    {
        uint64_t key = node.keys[i];
        if (!isBigEndian())
        {
            key = reverseBytes(key);
        }
        file.write((char *)&key, 8);
    }

    for (int i = 0; i < 19; i++)
    {
        uint64_t value = node.values[i];
        if (!isBigEndian())
        {
            value = reverseBytes(value);
        }
        file.write((char *)&value, 8);
    }

    for (int i = 0; i < 20; i++)
    {
        uint64_t offset = node.offsets[i];
        if (!isBigEndian())
        {
            offset = reverseBytes(offset);
        }
        file.write((char *)&offset, 8);
    }

    char zeros[8] = {0};
    file.write(zeros, 8);

    file.close();
    return true;
}

Node createEmptyNode(uint64_t blockID, uint64_t parentID, bool isLeaf)
{
    Node node;
    node.blockID = blockID;
    node.parent_ID = parentID;
    node.currentPairs = 0;

    for (int i = 0; i < 19; i++)
    {
        node.keys[i] = 0;
        node.values[i] = 0;
    }

    for (int i = 0; i < 20; i++)
    {
        node.offsets[i] = 0;
    }

    return node;
}

bool isLeaf(Node &node)
{
    for (int i = 0; i < 20; i++)
    {
        if (node.offsets[i] != 0)
        {
            return false;
        }
    }
    return true;
}

int findInsertPosition(Node &node, uint64_t key)
{
    int pos = 0;
    while (pos < node.currentPairs && key > node.keys[pos])
    {
        pos++;
    }
    return pos;
}

void splitChild(std::string filename, Node &parent, int childIndex, uint64_t &nextBlockID)
{

    Node fullChild = loadNode(filename, parent.offsets[childIndex]);

    Node newNode = createEmptyNode(nextBlockID, parent.blockID, isLeaf(fullChild));
    nextBlockID++;

    int midIndex = 9;

    newNode.currentPairs = 9;
    for (int i = 0; i < 9; i++)
    {
        newNode.keys[i] = fullChild.keys[midIndex + 1 + i];
        newNode.values[i] = fullChild.values[midIndex + 1 + i];
    }

    if (!isLeaf(fullChild))
    {
        for (int i = 0; i < 10; i++)
        {
            newNode.offsets[i] = fullChild.offsets[midIndex + 1 + i];
        }

        for (int i = 0; i < 10; i++)
        {
            if (newNode.offsets[i] != 0)
            {
                Node child = loadNode(filename, newNode.offsets[i]);
                child.parent_ID = newNode.blockID;
                saveNode(filename, child);
            }
        }
    }

    fullChild.currentPairs = 9;

    for (int i = 9; i < 19; i++)
    {
        fullChild.keys[i] = 0;
        fullChild.values[i] = 0;
    }
    if (!isLeaf(fullChild))
    {
        for (int i = 10; i < 20; i++)
        {
            fullChild.offsets[i] = 0;
        }
    }

    for (int i = parent.currentPairs; i > childIndex; i--)
    {
        parent.keys[i] = parent.keys[i - 1];
        parent.values[i] = parent.values[i - 1];
        parent.offsets[i + 1] = parent.offsets[i];
    }

    parent.keys[childIndex] = fullChild.keys[midIndex];
    parent.values[childIndex] = fullChild.values[midIndex];
    parent.offsets[childIndex + 1] = newNode.blockID;
    parent.currentPairs++;

    saveNode(filename, fullChild);
    saveNode(filename, newNode);
    saveNode(filename, parent);
}

void insertNonFull(std::string filename, Node &node, uint64_t key, uint64_t value, uint64_t &nextBlockID)
{
    int i = node.currentPairs - 1;

    if (isLeaf(node))
    {
        while (i >= 0 && key < node.keys[i])
        {
            node.keys[i + 1] = node.keys[i];
            node.values[i + 1] = node.values[i];
            i--;
        }

        node.keys[i + 1] = key;
        node.values[i + 1] = value;
        node.currentPairs++;

        saveNode(filename, node);
    }
    else
    {
        while (i >= 0 && key < node.keys[i])
        {
            i--;
        }
        i++;

        Node child = loadNode(filename, node.offsets[i]);

        if (child.currentPairs == 19)
        {
            splitChild(filename, node, i, nextBlockID);

            node = loadNode(filename, node.blockID);

            if (key > node.keys[i])
            {
                i++;
            }

            child = loadNode(filename, node.offsets[i]);
        }

        insertNonFull(filename, child, key, value, nextBlockID);
    }
}

bool insertKey(std::string filename, uint64_t key, uint64_t value)
{
    Header header;
    if (!readHeader(filename, header))
    {
        return false;
    }

    uint64_t rootID = header.rootID;
    uint64_t nextBlockID = header.nextBlockID;

    if (rootID == 0)
    {

        Node root = createEmptyNode(nextBlockID, 0, true);
        root.keys[0] = key;
        root.values[0] = value;
        root.currentPairs = 1;

        saveNode(filename, root);

        rootID = nextBlockID;
        nextBlockID++;
        writeHeader(filename, rootID, nextBlockID);

        return true;
    }

    Node root = loadNode(filename, rootID);

    if (root.currentPairs == 19)
    {
        Node newRoot = createEmptyNode(nextBlockID, 0, false);
        nextBlockID++;

        newRoot.offsets[0] = root.blockID;
        root.parent_ID = newRoot.blockID;
        saveNode(filename, root);

        splitChild(filename, newRoot, 0, nextBlockID);

        rootID = newRoot.blockID;
        writeHeader(filename, rootID, nextBlockID);

        newRoot = loadNode(filename, newRoot.blockID);
        insertNonFull(filename, newRoot, key, value, nextBlockID);

        writeHeader(filename, rootID, nextBlockID);
    }
    else
    {
        insertNonFull(filename, root, key, value, nextBlockID);

        writeHeader(filename, rootID, nextBlockID);
    }

    return true;
}

bool convertToUint64(const char *str, uint64_t &result)
{
    if (str == nullptr)
    {
        return false;
    }

    try
    {
        result = std::stoull(str);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool searchKey(std::string filename, uint64_t key, uint64_t &resultValue)
{
    Header header;
    if (!readHeader(filename, header))
    {
        return false;
    }

    if (header.rootID == 0)
    {
        return false;
    }

    uint64_t currentBlockID = header.rootID;

    while (currentBlockID != 0)
    {
        Node node = loadNode(filename, currentBlockID);

        int i = 0;
        while (i < node.currentPairs && key > node.keys[i])
        {
            i++;
        }

        if (i < node.currentPairs && key == node.keys[i])
        {
            resultValue = node.values[i];
            return true;
        }

        if (isLeaf(node))
        {
            return false;
        }

        currentBlockID = node.offsets[i];
    }

    return false;
}