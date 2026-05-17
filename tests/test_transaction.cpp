#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Transaction.h"
#include "mock_account.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::AnyNumber;

TEST(Transaction, Fee) {
    Transaction t;
    EXPECT_EQ(t.fee(), 1);
    t.set_fee(5);
    EXPECT_EQ(t.fee(), 5);
}

TEST(Transaction, Make_ThrowsOnSameId) {
    MockAccount a(1,100);
    Transaction t;
    EXPECT_THROW(t.Make(a,a,200), std::logic_error);
}

TEST(Transaction, Make_ThrowsOnNegativeSum) {
    MockAccount f(1,100), t(2,100);
    Transaction tx;
    EXPECT_THROW(tx.Make(f,t,-10), std::invalid_argument);
}

TEST(Transaction, Make_ThrowsOnTooSmallSum) {
    MockAccount f(1,100), t(2,100);
    Transaction tx;
    EXPECT_THROW(tx.Make(f,t,50), std::logic_error);
}

TEST(Transaction, Make_ReturnsFalseIfFeeTooHigh) {
    MockAccount f(1,1000), t(2,1000);
    Transaction tx;
    tx.set_fee(100);
    EXPECT_FALSE(tx.Make(f,t,150));
}

TEST(Transaction, Make_Success) {
    MockAccount from(1,1000), to(2,500);
    Transaction tx;
    tx.set_fee(10);
    int sum = 200;

    Sequence seq;
    EXPECT_CALL(from, Lock()).InSequence(seq);
    EXPECT_CALL(to, Lock()).InSequence(seq);
    EXPECT_CALL(to, ChangeBalance(sum)).InSequence(seq);
    EXPECT_CALL(to, GetBalance()).WillRepeatedly(Return(700));
    EXPECT_CALL(to, ChangeBalance(-(sum + tx.fee()))).InSequence(seq);
    EXPECT_CALL(to, Unlock()).InSequence(seq);
    EXPECT_CALL(from, Unlock()).InSequence(seq);

    EXPECT_TRUE(tx.Make(from, to, sum));
}

TEST(Transaction, Make_FailAndRollback) {
    MockAccount from(1,1000), to(2,0);
    Transaction tx;
    tx.set_fee(10);
    int sum = 100;

    Sequence seq;
    EXPECT_CALL(from, Lock()).InSequence(seq);
    EXPECT_CALL(to, Lock()).InSequence(seq);
    EXPECT_CALL(to, ChangeBalance(sum)).InSequence(seq);
    EXPECT_CALL(to, GetBalance()).WillRepeatedly(Return(100));
    EXPECT_CALL(to, ChangeBalance(-sum)).InSequence(seq); // откат
    EXPECT_CALL(to, Unlock()).InSequence(seq);
    EXPECT_CALL(from, Unlock()).InSequence(seq);

    EXPECT_FALSE(tx.Make(from, to, sum));
}

TEST(Transaction, Make_ExceptionRollback) {
    MockAccount from(1,1000), to(2,500);
    Transaction tx;
    EXPECT_CALL(from, Lock());
    EXPECT_CALL(to, Lock());
    EXPECT_CALL(to, ChangeBalance(200)).WillOnce(::testing::Throw(std::runtime_error("")));
    EXPECT_CALL(to, Unlock());
    EXPECT_CALL(from, Unlock());
    EXPECT_THROW(tx.Make(from, to, 200), std::runtime_error);
}
