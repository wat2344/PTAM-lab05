# Laboratory work V

Данная лабораторная работа посвещена изучению фреймворков для тестирования на примере GTest

## Tutorial

```bash
export GITHUB\_USERNAME=<имя\_пользователя>
alias gsed=sed
cd ${GITHUB\_USERNAME}/workspace
pushd .
source scripts/activate
git clone https://github.com/${GITHUB\_USERNAME}/lab04 projects/lab05
cd projects/lab05
git remote remove origin
git remote add origin https://github.com/${GITHUB\_USERNAME}/lab05
```

Подготовили пространство

```bash
mkdir third-party
git submodule add https://github.com/google/googletest third-party/gtest
cd third-party/gtest \&\& git checkout release-1.8.1 \&\& cd ../..
```

Добавлен gtest версии 1.8.1

```bash
git add third-party/gtest
git commit -m "added gtest framework"
```

Коммит в git

```bash
gsed -i '/option(BUILD\_EXAMPLES "Build examples" OFF)/a\\
option(BUILD\_TESTS "Build tests" OFF)
' CMakeLists.txt
```

Добавлена опция BUILD\_TESTS в CMakeLists.txt

```bash
cat >> CMakeLists.txt <<EOF

if(BUILD\_TESTS)
  enable\_testing()
  add\_subdirectory(third-party/gtest)
  file(GLOB \\${PROJECT\_NAME}\_TEST\_SOURCES tests/\*.cpp)
  add\_executable(check \\${\\${PROJECT\_NAME}\_TEST\_SOURCES})
  target\_link\_libraries(check \\${PROJECT\_NAME} gtest\_main)
  add\_test(NAME check COMMAND check)
endif()
EOF
```

Добавлен в конец CMakeLists.txt блок для сборки

```bash
mkdir tests
cat > tests/test1.cpp <<EOF
#include <print.hpp>

#include <gtest/gtest.h>

TEST(Print, InFileStream)
{
  std::string filepath = "file.txt";
  std::string text = "hello";
  std::ofstream out{filepath};

  print(text, out);
  out.close();

  std::string result;
  std::ifstream in{filepath};
  in >> result;

  EXPECT\_EQ(result, text);
}
EOF
```

Созданы папка tests и файл test1.cpp, проверяющий вывод в файл

```bash
cmake -H. -B\_build -DBUILD\_TESTS=ON
cmake --build \_build
cmake --build \_build --target test
```

Сборка CMake. Тут была проблема, решил так: git checkout release-1.12.1

```bash
\_build/check
cmake --build \_build --target test -- ARGS=--verbose
```

Тестируем работу

```bash
gsed -i 's/lab04/lab05/g' README.md
```

Заменяем все lab04 на lab05 в файле README.md

Дальше начинаются отклонения, так как замена Travis CI на GitHub Actions

```bash
mkdir -p .github/workflows
cat > .github/workflows/ci.yml <<EOF
name: CI

on: \[push, pull\_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
      with:
        submodules: true   # рекурсивно подтягивает submodule gtest

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake g++

    - name: Configure CMake
      run: cmake -H. -B\_build -DBUILD\_TESTS=ON

    - name: Build
      run: cmake --build \_build

    - name: Run tests (verbose)
      run: |
        cmake --build \_build --target test -- ARGS=--verbose
EOF
git add .github/workflows/ci.yml
```

Создаём workflows

```bash
git add tests
git add -p
git commit -m "added tests and GitHub Actions workflow"
git push origin master
```

Отправлены изменения на удалённый репозиторий

```bash
gh auth login
gh workflow run ci.yml --ref master
mkdir artifacts
sleep 20s \&\& gnome-screenshot --file artifacts/screenshot.png
```

Создана папка для скриншотов и сделан скриншот через 20 секунд

## Homework

```bash
mkdir ./src
mkdir ./tests
```

Это директории с файлами. В src перемещаем все исходные файлы.

```cmake
cmake\_minimum\_required(VERSION 3.10)
project(banking)

set(CMAKE\_CXX\_STANDARD 14)

include(FetchContent)
FetchContent\_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
FetchContent\_MakeAvailable(googletest)

add\_library(banking STATIC src/Account.cpp src/Transaction.cpp)
target\_include\_directories(banking PUBLIC src)

add\_executable(banking\_tests tests/test\_account.cpp tests/test\_transaction.cpp)

target\_link\_libraries(banking\_tests PRIVATE banking gtest gmock\_main)

enable\_testing()
add\_test(NAME BankingTests COMMAND banking\_tests)

option(ENABLE\_COVERAGE "Enable coverage" OFF)
if(ENABLE\_COVERAGE)
    target\_compile\_options(banking PRIVATE -O0 -g --coverage)
    target\_link\_libraries(banking PRIVATE --coverage)
    target\_compile\_options(banking\_tests PRIVATE -O0 -g --coverage)
    target\_link\_libraries(banking\_tests PRIVATE --coverage)
endif()
```

Это всё вставляем в корневой CMakeLists.txt через nano. Это CMake добавляет файлы, тесты и подключает GoogleTest и GoogleMock

```bash
cat > ./tests/mock\_account.h << EOF
#pragma once
#include <gmock/gmock.h>
#include "Account.h"

class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    MOCK\_METHOD(int, GetBalance, (), (const, override));
    MOCK\_METHOD(void, ChangeBalance, (int), (override));
    MOCK\_METHOD(void, Lock, (), (override));
    MOCK\_METHOD(void, Unlock, (), (override));
};
EOF
```

Файл для переопределения MOCK методов

```bash
cat > ./tests/test\_account.cpp << EOF
#include <gtest/gtest.h>
#include "Account.h"

TEST(Account, Basics) {
    Account a(1, 100);
    EXPECT\_EQ(a.id(), 1);
    EXPECT\_EQ(a.GetBalance(), 100);
    
    a.Lock();
    a.ChangeBalance(50);
    EXPECT\_EQ(a.GetBalance(), 150);
    
    EXPECT\_THROW(a.Lock(), std::runtime\_error);
    
    a.Unlock();
    EXPECT\_NO\_THROW(a.Lock());
    a.Unlock();
    
    EXPECT\_THROW(a.ChangeBalance(10), std::runtime\_error);
}
EOF
```

Тестирование без моков

```bash
cat > ./tests/test\_transaction.cpp << EOF
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Transaction.h"
#include "mock\_account.h"

using ::testing::\_;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::AnyNumber;

TEST(Transaction, Fee) {
    Transaction t;
    EXPECT\_EQ(t.fee(), 1);
    t.set\_fee(5);
    EXPECT\_EQ(t.fee(), 5);
}

TEST(Transaction, Make\_ThrowsOnSameId) {
    MockAccount a(1,100);
    Transaction t;
    EXPECT\_THROW(t.Make(a,a,200), std::logic\_error);
}

TEST(Transaction, Make\_ThrowsOnNegativeSum) {
    MockAccount f(1,100), t(2,100);
    Transaction tx;
    EXPECT\_THROW(tx.Make(f,t,-10), std::invalid\_argument);
}

TEST(Transaction, Make\_ThrowsOnTooSmallSum) {
    MockAccount f(1,100), t(2,100);
    Transaction tx;
    EXPECT\_THROW(tx.Make(f,t,50), std::logic\_error);
}

TEST(Transaction, Make\_ReturnsFalseIfFeeTooHigh) {
    MockAccount f(1,1000), t(2,1000);
    Transaction tx;
    tx.set\_fee(100);
    EXPECT\_FALSE(tx.Make(f,t,150));
}

TEST(Transaction, Make\_Success) {
    MockAccount from(1,1000), to(2,500);
    Transaction tx;
    tx.set\_fee(10);
    int sum = 200;

    Sequence seq;
    EXPECT\_CALL(from, Lock()).InSequence(seq);
    EXPECT\_CALL(to, Lock()).InSequence(seq);
    EXPECT\_CALL(to, ChangeBalance(sum)).InSequence(seq);
    EXPECT\_CALL(to, GetBalance()).WillRepeatedly(Return(700));
    EXPECT\_CALL(to, ChangeBalance(-(sum + tx.fee()))).InSequence(seq);
    EXPECT\_CALL(to, Unlock()).InSequence(seq);
    EXPECT\_CALL(from, Unlock()).InSequence(seq);

    EXPECT\_TRUE(tx.Make(from, to, sum));
}

TEST(Transaction, Make\_FailAndRollback) {
    MockAccount from(1,1000), to(2,0);
    Transaction tx;
    tx.set\_fee(10);
    int sum = 100;

    Sequence seq;
    EXPECT\_CALL(from, Lock()).InSequence(seq);
    EXPECT\_CALL(to, Lock()).InSequence(seq);
    EXPECT\_CALL(to, ChangeBalance(sum)).InSequence(seq);
    EXPECT\_CALL(to, GetBalance()).WillRepeatedly(Return(100));
    EXPECT\_CALL(to, ChangeBalance(-sum)).InSequence(seq); // откат
    EXPECT\_CALL(to, Unlock()).InSequence(seq);
    EXPECT\_CALL(from, Unlock()).InSequence(seq);

    EXPECT\_FALSE(tx.Make(from, to, sum));
}

TEST(Transaction, Make\_ExceptionRollback) {
    MockAccount from(1,1000), to(2,500);
    Transaction tx;
    EXPECT\_CALL(from, Lock());
    EXPECT\_CALL(to, Lock());
    EXPECT\_CALL(to, ChangeBalance(200)).WillOnce(::testing::Throw(std::runtime\_error("")));
    EXPECT\_CALL(to, Unlock());
    EXPECT\_CALL(from, Unlock());
    EXPECT\_THROW(tx.Make(from, to, 200), std::runtime\_error);
}
EOF
```

Тестирование с моками

```bash
cat > .github/workflows/ci.yml << EOF
name: CI

on: \[push, pull\_request]

jobs:
  coverage:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install gcovr
        run: sudo apt update \&\& sudo apt install -y gcovr

      - name: Configure CMake with coverage
        run: |
          rm -rf build
          cmake -B build -DENABLE\_COVERAGE=ON

      - name: Build
        run: cmake --build build

      - name: Run tests
        run: cd build \&\& ctest --output-on-failure

      - name: Generate coverage report (cobertura)
        run: |
          cd build
          gcovr --gcov-executable gcov --root .. --cobertura --output coverage.xml

      - name: Upload to Coveralls
        uses: coverallsapp/github-action@v2
        with:
          github-token: \\${{ secrets.GITHUB\_TOKEN }}
          file: build/coverage.xml
          format: cobertura
EOF
```

Создан .yml. Это финальная версия, которая получилась при решении ошибок на гитхабе

```bash
mkdir build
cd build
cmake -DENABLE\_COVERAGE=ON ..
make
```

Сборка CMakeLists

```bash
ctest
```

Проведён тест, успешно.

Вывод:

```
Test project /home/vboxuser/wat2344/workspace/projects/PTAM-lab05/build
    Start 1: BankingTests
1/1 Test #1: BankingTests .....................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.01 sec
```

Далее шла настройка Coveralls. Там возникало много ошибок при сборке, но по итогу Coveralls показал репозиторий

```

