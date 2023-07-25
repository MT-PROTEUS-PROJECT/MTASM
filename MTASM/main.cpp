#include <ASM/ASM.h>
#include <ASM/Exceptions.h>
#include <Utils/Logger.h>

#include <fstream>

#ifdef _DEBUG
constexpr const char *file = "TestFile.txt";
#endif

#include <Windows.h>
#undef ERROR

int main(int argc, char **argv)
{
#ifndef _DEBUG
    ShowWindow(GetConsoleWindow(), SW_HIDE);
#else
    ShowWindow(GetConsoleWindow(), SW_SHOW);
#endif
    setlocale(LC_ALL, "Russian");

    std::ifstream in;
    if (argc == 2)
    {
        in.open(argv[1], std::ios::in);
    }
    else
    {
#ifdef _DEBUG
        in.open(file, std::ios::in);
#else
        throw std::runtime_error("Неверное число аргументов!\n Usage: MTASM.exe path_to_file.txt");
#endif
    }

    if (!in.is_open())
        throw std::runtime_error("Не удалось открыть файл для чтения");

    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);
    
    yy::ASM mtasm(in);
    int res = mtasm.Parse();
    LOG(INFO) << "Parse result: " << res << "\n";

    auto se = mtasm.GetEC().Get(ExceptionContainer::Tag::SE);
    auto ice = mtasm.GetEC().Get(ExceptionContainer::Tag::ICE);
    auto other = mtasm.GetEC().Get(ExceptionContainer::Tag::OTHER);
    if (!se.empty())
    {
        LOG(ERROR) << "Синтаксические ошибки:\n";
        for (const auto &ex : se)
            LOG(ERROR) << "\t" << ex._msg << "\n";
    }
    if (!ice.empty())
    {
        LOG(FATAL) << "Ошибки компилятора:\n";
        for (const auto &ex : ice)
            LOG(FATAL) << "\t" << ex._msg << "\n";
    }
    if (!other.empty())
    {
        LOG(FATAL) << "Непредвиденные ошибки:\n";
        for (const auto &ex : other)
            LOG(FATAL) << "\t" << ex._msg << "\n";
    }

    return 0;
}
