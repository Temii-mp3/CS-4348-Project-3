#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <string.h>
#define MAGIC "4348PRJ3"

int isBigEndian();
uint64_t reverseBytes(uint64_t x);
bool isIndexFile(std::string, const char *);
bool readHeader(std::string, Header);

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

struct Header
{
    char magic[8];
    uint64_t rootID;
    uint64_t nextBlockID;
};

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
        uint64_t key = (uint64_t)argv[3];
        uint64_t value = (uint64_t)argv[4];
        if (indexFile.empty())
        {
            std::cerr << "Usage: " << command << " insert <index-file> <key> <value>" << std::endl;
            return 1;
        }
        if (!isIndexFile(indexFile, MAGIC))
        {
            std::cerr << "Invalid file, file must be an index file";
        }

        std::cout << "You want to insert key " << key << " with value " << value << " using filename " << indexFile << std::endl;
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
        uint64_t key = (uint64_t)argv[3];
        if (indexFile.empty())
        {
            std::cerr << "Usage: " << command << " search <index-file> <key>" << std::endl;
            return 1;
        }
        if (!isIndexFile(indexFile, MAGIC))
        {
            std::cerr << "Invalid file, file must be an index file";
        }
    }
    else if (command == "load")
    {
        uint64_t key = (uint64_t)argv[3];
        std::string csvFile = argv[4];
        if (indexFile.empty() || csvFile.empty())
        {
            std::cerr << "Usage: " << command << " load <index-file> <csv-file>" << std::endl;
            return 1;
        }
        if (!isIndexFile(indexFile, MAGIC))
        {
            std::cerr << "Invalid file, file must be an index file";
        }
    }
    else if (command == "print")
    {
        if (indexFile.empty())
        {
            std::cerr << "Usage: " << command << " insert <index-file> <key> <value>" << std::endl;
            return 1;
        }
        if (!isIndexFile(indexFile, MAGIC))
        {
            std::cerr << "Invalid file, file must be an index file";
        }
    }
    else if (command == "extract")
    {
        uint64_t key = (uint64_t)argv[3];
        std::string csvFile = argv[4];
        if (indexFile.empty() || csvFile.empty())
        {
            std::cerr << "Usage: " << command << " load <index-file> <csv-file>" << std::endl;
            return 1;
        }
        if (!isIndexFile(indexFile, MAGIC))
        {
            std::cerr << "Invalid file, file must be an index file";
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
        return 1;
    }
    char buffer[8];

    infile.read(buffer, 8);

    if (strncmp(buffer, MAGIC, 8) == 0)
    {
        return true;
    }

    return false;
}

bool insert(std::string indexFile, uint64_t key, uint64_t value)
{
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

    file.seekg(blockID * 512, std::ios::beg); // find location of node

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