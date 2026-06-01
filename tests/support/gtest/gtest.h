#pragma once

#include <QDebug>
#include <QString>

#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace testing {

class FatalFailure final : public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "fatal test assertion";
    }
};

struct TestCase {
    const char* suite;
    const char* name;
    void (*body)();
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

class Registrar final {
public:
    Registrar(const char* suite, const char* name, void (*body)()) {
        registry().push_back({suite, name, body});
    }
};

inline int failureCount = 0;
inline QString currentMessage;

template <typename T>
QString printable(const T& value) {
    QString out;
    QDebug debug(&out);
    debug.noquote() << value;
    return out.trimmed();
}

inline QString printable(const std::string& value) {
    return QString::fromStdString(value);
}

inline QString printable(const char* value) {
    return QString::fromUtf8(value ? value : "");
}

class AssertionStream final {
public:
    AssertionStream(bool failed, bool fatal, const char* file = "", int line = 0, QString message = {})
        : m_failed(failed)
        , m_fatal(fatal) {
        if (m_failed) {
            currentMessage = QStringLiteral("%1:%2: %3").arg(QString::fromUtf8(file)).arg(line).arg(message);
        }
    }

    AssertionStream(AssertionStream&& other) noexcept
        : m_failed(std::exchange(other.m_failed, false))
        , m_fatal(other.m_fatal) {}

    AssertionStream(const AssertionStream&) = delete;
    AssertionStream& operator=(const AssertionStream&) = delete;

    ~AssertionStream() noexcept(false) {
        if (!m_failed) {
            return;
        }
        ++failureCount;
        std::cerr << currentMessage.toStdString() << '\n';
        if (m_fatal) {
            throw FatalFailure{};
        }
    }

    template <typename T>
    AssertionStream& operator<<(const T& value) {
        if (m_failed) {
            currentMessage += QStringLiteral(" ");
            currentMessage += printable(value);
        }
        return *this;
    }

private:
    bool m_failed;
    bool m_fatal;
};

inline void InitGoogleTest(int*, char**) {}

inline int RunAllTests() {
    int failedTests = 0;
    for (const auto& test : registry()) {
        const int before = failureCount;
        std::cout << "[ RUN      ] " << test.suite << "." << test.name << '\n';
        try {
            test.body();
        } catch (const FatalFailure&) {
        } catch (const std::exception& error) {
            ++failureCount;
            std::cerr << "Unhandled exception: " << error.what() << '\n';
        } catch (...) {
            ++failureCount;
            std::cerr << "Unhandled non-standard exception\n";
        }

        if (failureCount == before) {
            std::cout << "[       OK ] " << test.suite << "." << test.name << '\n';
        } else {
            ++failedTests;
            std::cout << "[  FAILED  ] " << test.suite << "." << test.name << '\n';
        }
    }
    std::cout << "[==========] " << registry().size() << " tests ran\n";
    return failedTests == 0 ? 0 : 1;
}

inline AssertionStream checkTrue(bool value, bool fatal, const char* file, int line, const char* expr) {
    if (value) {
        return AssertionStream(false, fatal);
    }
    return AssertionStream(true, fatal, file, line, QStringLiteral("Expected true: %1").arg(QString::fromUtf8(expr)));
}

inline AssertionStream checkFalse(bool value, bool fatal, const char* file, int line, const char* expr) {
    if (!value) {
        return AssertionStream(false, fatal);
    }
    return AssertionStream(true, fatal, file, line, QStringLiteral("Expected false: %1").arg(QString::fromUtf8(expr)));
}

template <typename Left, typename Right>
AssertionStream checkEq(const Left& left,
                        const Right& right,
                        bool fatal,
                        const char* file,
                        int line,
                        const char* leftExpr,
                        const char* rightExpr) {
    if (left == right) {
        return AssertionStream(false, fatal);
    }
    return AssertionStream(true,
                           fatal,
                           file,
                           line,
                           QStringLiteral("Expected equality of %1 and %2, actual %3 vs %4")
                               .arg(QString::fromUtf8(leftExpr),
                                    QString::fromUtf8(rightExpr),
                                    printable(left),
                                    printable(right)));
}

template <typename Left, typename Right>
AssertionStream checkGt(const Left& left,
                        const Right& right,
                        bool fatal,
                        const char* file,
                        int line,
                        const char* leftExpr,
                        const char* rightExpr) {
    if (left > right) {
        return AssertionStream(false, fatal);
    }
    return AssertionStream(true,
                           fatal,
                           file,
                           line,
                           QStringLiteral("Expected %1 > %2, actual %3 vs %4")
                               .arg(QString::fromUtf8(leftExpr),
                                    QString::fromUtf8(rightExpr),
                                    printable(left),
                                    printable(right)));
}

}  // namespace testing

#define TEST(SUITE, NAME)                                                                         \
    static void SUITE##_##NAME##_body();                                                          \
    static ::testing::Registrar SUITE##_##NAME##_registrar(#SUITE, #NAME, &SUITE##_##NAME##_body); \
    static void SUITE##_##NAME##_body()

#define EXPECT_TRUE(EXPR)                                                                                   \
    if (static_cast<bool>(EXPR)) {                                                                          \
    } else                                                                                                  \
        ::testing::AssertionStream(true, false, __FILE__, __LINE__, QStringLiteral("Expected true: " #EXPR))

#define ASSERT_TRUE(EXPR)                                                                                  \
    if (static_cast<bool>(EXPR)) {                                                                         \
    } else                                                                                                 \
        ::testing::AssertionStream(true, true, __FILE__, __LINE__, QStringLiteral("Expected true: " #EXPR))

#define EXPECT_FALSE(EXPR)                                                                                    \
    if (!static_cast<bool>(EXPR)) {                                                                           \
    } else                                                                                                    \
        ::testing::AssertionStream(true, false, __FILE__, __LINE__, QStringLiteral("Expected false: " #EXPR))
#define ASSERT_EQ(LEFT, RIGHT) ::testing::checkEq((LEFT), (RIGHT), true, __FILE__, __LINE__, #LEFT, #RIGHT)
#define EXPECT_EQ(LEFT, RIGHT) ::testing::checkEq((LEFT), (RIGHT), false, __FILE__, __LINE__, #LEFT, #RIGHT)
#define ASSERT_GT(LEFT, RIGHT) ::testing::checkGt((LEFT), (RIGHT), true, __FILE__, __LINE__, #LEFT, #RIGHT)
#define RUN_ALL_TESTS() ::testing::RunAllTests()
