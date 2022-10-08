#pragma once
#include <string>
#include <sstream>
#include <stdexcept>
#include <glad/glad.h>

// Returns the name of the file in which it is called.
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

// Crash if the condition is false.
#define Assert(condition, message) do { Core::Debug::Assertion::DoAssertion(condition, __FILENAME__, __FUNCTION__, __LINE__, message, false); if (!(condition)) throw std::runtime_error(message); } while(0)

// Log info to console and log file cache at the same time.
#define DebugLog(message) Core::Debug::Log::ToFile(Core::Debug::LogTypes::Info, __FILENAME__, __FUNCTION__, __LINE__, message, true)
// Log warning to console and log file cache at the same time.
#define DebugLogWarning(message) Core::Debug::Log::ToFile(Core::Debug::LogTypes::Warning, __FILENAME__, __FUNCTION__, __LINE__, message, true)
// Log error to console and log file cache at the same time.
#define DebugLogError(message) Core::Debug::Log::ToFile(Core::Debug::LogTypes::Error, __FILENAME__, __FUNCTION__, __LINE__, message, true)
// Log of specified log type in console and log file cache at the same time.
#define DebugLogWithType(type, message) Core::Debug::Log::ToFile(type, __FILENAME__, __FUNCTION__, __LINE__, message, true)

// Set the file where logs will be saved.
#define DebugSetLogFile(filename) Core::Debug::Log::SetLogFilename(filename)
// Set and clear the file where logs will be saved.
#define DebugSetAndClearLogFile(filename) Core::Debug::Log::SetLogFilename(filename, true)
// Save logs to the set file.
#define DebugSaveLogFile() delete Core::Debug::Log::logFileCache;

// Log only to the console.
#define DebugLogToConsole(type, message) Core::Debug::Log::ToConsole(type, __FILENAME__, __FUNCTION__, __LINE__, message)
// Add log only to file cache.
#define DebugLogToFile(type, message) Core::Debug::Log::ToFile(type, __FILENAME__, __FUNCTION__, __LINE__, message)

// OpenGL log callback.
void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

namespace Core::Debug
{
    enum class LogTypes
    {
        Info,
        Warning,
        Error
    };

    class Assertion
    {
    public:
        static void DoAssertion(const bool& condition, const char* sourceFile, 
                                const char* sourceFunction, const long& sourceLine, 
                                const std::string& message, const bool& throwError = true);
    };

    class LogFileCache
    {
    public:
		// Use to lock / unlock code from threads.
		std::atomic_flag lock = ATOMIC_FLAG_INIT;

        const char* filename;
        std::stringstream fileCache;

        LogFileCache(const char* file, bool clearFile = false);
        ~LogFileCache();
        void Append(const std::string& message);
    };

    class Log
    {
    private:
		// Use to lock / unlock code from threads.
		static std::atomic_flag lock;

    public:
        static LogFileCache* logFileCache;

        static void SetLogFilename(const char* filename, bool clearFile = false);
        static void ToConsole(const LogTypes& type, const char* sourceFile, 
                              const char* sourceFunction, const long& sourceLine, 
                              const std::string& message);
        static void ToFile   (const LogTypes& type, const char* sourceFile, 
                              const char* sourceFunction, const long& sourceLine, 
                              const std::string& message, const bool& logToConsole = false);

    private:
        static std::string FormatLog(const LogTypes& type, const char* sourceFile, 
                                     const char* sourceFunction, const long& sourceLine, 
                                     const std::string& message);
        static std::string LogTypeToStr(const LogTypes& type);
    };
}