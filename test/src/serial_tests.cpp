//
// Created by Konstantin Gredeskoul on 5/16/17.
//
#include <serial.h>
#include "gtest/gtest.h"

using namespace std;

class HuffmanTest : public ::testing::Test {

protected:

  virtual void SetUp() {
  };

  virtual void TearDown() {
  };

  virtual void verify() {
  }
};

TEST_F(HuffmanTest, TEST_NAME) {
  verify(/*SOME TEST VARIABLE*/);
}

