#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>

/**
 * @brief Batch command processor.
 */

struct Command
{
    std::string Text;
    std::chrono::system_clock::time_point Timestamp;
};

class CommandProcessor
{
public:
    CommandProcessor(CommandProcessor* nextCommandProcessor = nullptr)
        : mNextCommandProcessor(nextCommandProcessor)
    {
    }

    virtual ~CommandProcessor() = default;

    virtual void StartBlock() {}
    virtual void FinishBlock() {}

    virtual void ProcessCommand(const Command& command) = 0;

protected:
    CommandProcessor* mNextCommandProcessor;
};

class ConsoleInput : public CommandProcessor
{
public:
    ConsoleInput(CommandProcessor* nextCommandProcessor = nullptr)
        : CommandProcessor(nextCommandProcessor)
        , mBlockDepth(0)
    {
    }

    void ProcessCommand(const Command& command) override
    {
        if (mNextCommandProcessor)
        {
            if (command.Text == "{")
            {
                if (mBlockDepth++ == 0)
                    mNextCommandProcessor->StartBlock();
            }
            else if (command.Text == "}")
            {
                if (--mBlockDepth == 0)
                    mNextCommandProcessor->FinishBlock();
            }
            else
                mNextCommandProcessor->ProcessCommand(command);
        }
    }

private:
    int mBlockDepth;
};

class ConsoleOutput : public CommandProcessor
{
public:
    ConsoleOutput(CommandProcessor* nextCommandProcessor = nullptr)
        : CommandProcessor(nextCommandProcessor)
    {
    }

    void ProcessCommand(const Command& command) override
    {
        std::cout << command.Text << std::endl;

        if (mNextCommandProcessor)
            mNextCommandProcessor->ProcessCommand(command);
    }
};

class ReportWriter : public CommandProcessor
{
public:
    ReportWriter(CommandProcessor* nextCommandProcessor = nullptr)
        : CommandProcessor(nextCommandProcessor)
    {
    }

    void ProcessCommand(const Command& command) override
    {
        std::ofstream file(GetFilename(command), std::ofstream::out);
        file << command.Text;

        if (mNextCommandProcessor)
            mNextCommandProcessor->ProcessCommand(command);
    }

private:
    std::string GetFilename(const Command& command)
    {
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
        command.Timestamp.time_since_epoch()).count();
        std::stringstream filename;
        filename << "bulk" << seconds << ".log";
        return filename.str();
    }
};

class BatchCommandProcessor : public CommandProcessor
{
public:
    BatchCommandProcessor(int bulkSize, CommandProcessor* nextCommandProcessor)
        : CommandProcessor(nextCommandProcessor)
        , mBulkSize(bulkSize)
        , mBlockForced(false)
    {
    }

    ~BatchCommandProcessor()
    {
        if (!mBlockForced)
            DumpBatch();
    }

    void StartBlock() override
    {
        mBlockForced = true;
        DumpBatch();
    }

    void FinishBlock() override
    {
        mBlockForced = false;
        DumpBatch();
    }

    void ProcessCommand(const Command& command) override
    {
        mCommandBatch.push_back(command);

        if (!mBlockForced && mCommandBatch.size() >= mBulkSize)
        {
            DumpBatch();
        }
    }
private:
    void ClearBatch()
    {
        mCommandBatch.clear();
    }

    void DumpBatch()
    {
        if (mNextCommandProcessor && !mCommandBatch.empty())
        {
            std::string output = "bulk: " + Join(mCommandBatch);
            mNextCommandProcessor->ProcessCommand(Command{output, mCommandBatch[0].Timestamp});
        }
        ClearBatch();
    }

    static std::string Join(const std::vector<Command>& v)
    {
        std::stringstream ss;
        for(size_t i = 0; i < v.size(); ++i)
        {
            if(i != 0)
                ss << ", ";
            ss << v[i].Text;
        }
        return ss.str();
    }
    int mBulkSize;
    bool mBlockForced;
    std::vector<Command> mCommandBatch;
};

void RunBulk(int bulkSize)
{
    ReportWriter reportWriter;
    ConsoleOutput consoleOutput(&reportWriter);
    BatchCommandProcessor batchCommandProcessor(bulkSize, &consoleOutput);
    ConsoleInput consoleInput(&batchCommandProcessor);
    std::string text;
    while (std::getline(std::cin, text))
        consoleInput.ProcessCommand(Command{text, std::chrono::system_clock::now()});
}

int main(int argc, char const** argv)
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Bulk size is not specified." << std::endl;
            return 1;
        }

        int bulkSize = atoi(argv[1]);
        if (bulkSize == 0)
        {
            std::cerr << "Invalid bulk size." << std::endl;
            return 1;
        }

        RunBulk(bulkSize);
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
