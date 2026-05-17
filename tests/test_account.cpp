#include <gtest/gtest.h>
#include "Account.h"

TEST(Account, Basics) {
    Account a(1, 100);
    EXPECT_EQ(a.id(), 1);
    EXPECT_EQ(a.GetBalance(), 100);
    
    a.Lock();
    a.ChangeBalance(50);
    EXPECT_EQ(a.GetBalance(), 150);
    
    EXPECT_THROW(a.Lock(), std::runtime_error);
    
    a.Unlock();
    EXPECT_NO_THROW(a.Lock());
    a.Unlock();
    
    EXPECT_THROW(a.ChangeBalance(10), std::runtime_error);
}
