#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <string.h>
int isBigEndian();
uint64_t reverseBytes(uint64_t x);
bool isIndexFile(std::string, const char *);

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
    const char *MAGIC = "4348PRJ3";

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

bool isIndexFile(std::string filename, const char *magic)
{
    std::ifstream infile(filename, std::ios::binary);
    if (!infile)
    {
        std::cerr << "File does not exist, please create it first" << std::endl;
        return 1;
    }
    char buffer[8];

    infile.read(buffer, 8);

    if (strncmp(buffer, magic, 8) == 0)
    {
        return true;
    }

    return false;
}

bool insert(std::string indexFile, uint64_t key, uint64_t value)
{
    char buff[8];
    std::ifstream infile(indexFile, std::ios::binary);
    infile.clear();
    infile.seekg(8, std::ios::beg);
    infile.read(buff, 8);

    if ((uint64_t)buff == 0)
    {
        Node node1;
        possibleNodes[0] = node1;
        possibleNodes[0].blockID = 1;
        possibleNodes[0].parent_ID = 0;

        rootID = 1;
        nextBlockID += 1;
    }
}