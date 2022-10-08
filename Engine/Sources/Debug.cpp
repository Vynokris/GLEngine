#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "Debug.h"
using namespace Core::Debug;


void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    // Ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::stringstream sstream;

    sstream << "\nDebug message (id " << id << "): " << message << "\n";

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             sstream << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sstream << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: sstream << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     sstream << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     sstream << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           sstream << "Source: Other"; break;
    } sstream << "\n";

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               sstream << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: sstream << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  sstream << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         sstream << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         sstream << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              sstream << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          sstream << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           sstream << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               sstream << "Type: Other"; break;
    } sstream << "\n";

    LogTypes logType;
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         sstream << "Severity: High";         logType = LogTypes::Error;   break;
    case GL_DEBUG_SEVERITY_MEDIUM:       sstream << "Severity: Medium";       logType = LogTypes::Error;   break;
    case GL_DEBUG_SEVERITY_LOW:          sstream << "Severity: Low";          logType = LogTypes::Warning; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: sstream << "Severity: Notification"; logType = LogTypes::Info;    break;
    } sstream << "\n";

    DebugLogWithType(logType, sstream.str().c_str());
}


// ----- Assertion ----- //

void Assertion::DoAssertion(const bool& condition, const char* sourceFile, const char* sourceFunction, const long& sourceLine, const std::string& message, const bool& throwError)
{
    if (!condition)
    {
        Log::ToFile(LogTypes::Error, sourceFile, sourceFunction, sourceLine, std::string("Assertion failed, ") + message, true);
        DebugSaveLogFile();
        if (throwError)
            throw std::runtime_error(message);
    }
}


// ----- Log file cache ----- //

LogFileCache::LogFileCache(const char* file, bool clearFile)
{
    filename = file;
    if (clearFile)
    {
        std::ofstream file(filename);
        file.close();
    }
}

LogFileCache::~LogFileCache()
{
    // Log to specified file.
    std::fstream f(filename, std::fstream::in | std::fstream::out | std::fstream::app);
    if (f.is_open())
    {
        f << fileCache.str();
        f.close();
    }
    else
    {
        DebugLogWarning((std::string("Unable to open file: ") + filename).c_str());
    }
}

void LogFileCache::Append(const std::string& message)
{
	while (lock.test_and_set()) {}
    fileCache << message;
	lock.clear();
}


// ----- Log ----- //

std::atomic_flag Log::lock = ATOMIC_FLAG_INIT;
LogFileCache*    Log::logFileCache = nullptr;

void Log::SetLogFilename(const char* filename, bool clearFile)
{
    // Delete the current file cache and create a new one.
    if (logFileCache == nullptr || filename != logFileCache->filename)
    {
        if (logFileCache != nullptr)
            delete logFileCache;
        logFileCache = new LogFileCache(filename, clearFile);
    }
}

std::string Log::FormatLog(const LogTypes& type, const char* sourceFile, const char* sourceFunction, const long& sourceLine, const std::string& message)
{
    std::string output = "\n";
    output += LogTypeToStr(type);
    output += std::filesystem::path(sourceFile).filename().string().c_str();
    if (sourceLine >= 0) {
        output += "(";
        output += std::to_string(sourceLine);
        output += ")";
    }
    if (sourceFunction[0] != '\0') {
        output += " in ";
        output += sourceFunction;
    }
    output += "\n";
    output += message;
    output += "\n";
    return output;
}

void Log::ToConsole(const LogTypes& type, const char* sourceFile, const char* sourceFunction, const long& sourceLine, const std::string& message)
{
	while (lock.test_and_set()) {}
    std::cout << FormatLog(type, sourceFile, sourceFunction, sourceLine, message);
	lock.clear();
}

void Log::ToFile(const LogTypes& type, const char* sourceFile, const char* sourceFunction, const long& sourceLine, const std::string& message, const bool& logToConsole)
{
    // Log to console if specified.
    if (logToConsole)
        ToConsole(type, sourceFile, sourceFunction, sourceLine, message);
    
    // Append to the current file cache.
    if (logFileCache != nullptr)
        logFileCache->Append(FormatLog(type, sourceFile, sourceFunction, sourceLine, message).c_str());
    else
        DebugLogToConsole(LogTypes::Warning, "Unable to log to file: filename not set.");
}


std::string Log::LogTypeToStr(const LogTypes& type)
{
    std::string msgType;
    switch (type)
    {
    case LogTypes::Info:
        msgType = "INFO: ";
        break;
    case LogTypes::Warning:
        msgType = "WARNING: ";
        break;
    case LogTypes::Error:
        msgType = "ERROR: ";
        break;
    default:
        break;
    }
    return msgType;
}