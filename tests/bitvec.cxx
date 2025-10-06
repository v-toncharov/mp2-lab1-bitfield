#include "bitvec.hxx"
#include <gtest.h>

// NOLINTBEGIN
TEST(bitvec, create_with_positive_length)
    { ASSERT_NO_THROW(bitvec<> bv(3)); }

TEST(bitvec, get_length) {
    bitvec<> bv(3);
    EXPECT_EQ(3, bv.len());
}

TEST(bitvec, new_bitvec_is_set_to_zero) {
    bitvec<> bv(100);
    unsigned sum = 0;
    for (size_t i = 0; i < bv.len(); i++) sum += bv[i];
    EXPECT_EQ(0, sum);
}

TEST(bitvec, set_bit) {
    bitvec<> bv(10);
    EXPECT_EQ(0, bv[3]);
    bv[3] = true;
    EXPECT_NE(0, bv[3]);
}

TEST(bitvec, clear_bit) {
    bitvec<> bv(10);
    bv[3] = true;
    EXPECT_NE(0, bv[3]);
    bv[3] = false;
    EXPECT_EQ(0, bv[3]);
}

TEST(bitvec, throws_when_create_bitfield_with_negative_length)
    { ASSERT_ANY_THROW(bitvec<> bv(-3)); }

TEST(bitvec, throws_when_set_bit_with_negative_index) {
    bitvec<> bv(10);
    ASSERT_ANY_THROW(bv[-3] = true);
}
TEST(bitvec, throws_when_set_bit_with_too_large_index) {
    bitvec<> bv(10);
    ASSERT_ANY_THROW(bv[11] = true);
}

TEST(bitvec, throws_when_get_bit_with_negative_index) {
    bitvec<> bv(10);
    ASSERT_ANY_THROW(bv[-3]);
}
TEST(bitvec, throws_when_get_bit_with_too_large_index) {
    bitvec<> bv(10);
    ASSERT_ANY_THROW(bv[11]);
}

TEST(bitvec, throws_when_clear_bit_with_negative_index) {
    bitvec<> bv(10);
    ASSERT_ANY_THROW(bv[-3] = false);
}
TEST(bitvec, throws_when_clear_bit_with_too_large_index) {
    bitvec<> bv(10);
    ASSERT_ANY_THROW(bv[11] = false);
}

TEST(bitvec, can_assign_bitfields_of_equal_size) {
    const unsigned size = 2;
    bitvec<> bv1(size), bv2(size);
    for (unsigned i = 0; i < size; i++) bv1[i] = true;
    bv2 = bv1;
    EXPECT_NE(0, bv2[0]);
    EXPECT_NE(0, bv2[1]);
}

TEST(bitvec, assign_operator_changes_bitfield_size) {
    const unsigned size1 = 2, size2 = 5;
    bitvec<> bv1(size1), bv2(size2);
    for (unsigned i = 0; i < size1; i++) bv1[i] = true;
    bv2 = bv1;
    EXPECT_EQ(2, bv2.len());
}

TEST(bitvec, can_assign_bitfields_of_non_equal_size) {
    const unsigned size1 = 2, size2 = 5;
    bitvec<> bv1(size1), bv2(size2);
    for (unsigned i = 0; i < size1; i++) bv1[i] = true;
    bv2 = bv1;
    EXPECT_NE(0, bv2[0]);
    EXPECT_NE(0, bv2[1]);
}

TEST(bitvec, compare_equal_bitfields_of_equal_size) {
    const unsigned size = 2;
    bitvec<> bv1(size), bv2(size);
    for (unsigned i = 0; i < size; i++) bv1[i] = true;
    bv2 = bv1;
    EXPECT_EQ(bv1, bv2);
}

TEST(bitvec, or_operator_applied_to_bitfields_of_equal_size) {
    const unsigned size = 4;
    bitvec<> bv1(size), bv2(size), expbv(size);
    // bv1 = 0011
    bv1[2] = true;
    bv1[3] = true;
    // bv2 = 0101
    bv2[1] = true;
    bv2[3] = true;

    // expbv = 0111
    expbv[1] = true;
    expbv[2] = true;
    expbv[3] = true;

    EXPECT_EQ(expbv, bv1 | bv2);
}

TEST(bitvec, or_operator_applied_to_bitfields_of_non_equal_size) {
    const unsigned size1 = 4, size2 = 5;
    bitvec<> bv1(size1), bv2(size2), expbv(size2);
    // bv1 = 0011
    bv1[2] = true;
    bv1[3] = true;
    // bv2 = 01010
    bv2[1] = true;
    bv2[3] = true;

    // expbv = 01110
    expbv[1] = true;
    expbv[2] = true;
    expbv[3] = true;

    EXPECT_EQ(expbv, bv1 | bv2);
}

TEST(bitvec, and_operator_applied_to_bitfields_of_equal_size) {
    const unsigned size = 4;
    bitvec<> bv1(size), bv2(size), expbv(size);
    // bv1 = 0011
    bv1[2] = true;
    bv1[3] = true;
    // bv2 = 0101
    bv2[1] = true;
    bv2[3] = true;

    // expbv = 0001
    expbv[3] = true;

    EXPECT_EQ(expbv, bv1 & bv2);
}

TEST(bitvec, and_operator_applied_to_bitfields_of_non_equal_size) {
    const unsigned size1 = 4, size2 = 5;
    bitvec<> bv1(size1), bv2(size2), expbv(size2);
    // bv1 = 0011
    bv1[2] = true;
    bv1[3] = true;
    // bv2 = 01010
    bv2[1] = true;
    bv2[3] = true;

    // expbv = 00010
    expbv[3] = true;

    EXPECT_EQ(expbv, bv1 & bv2);
}

TEST(bitvec, can_invert_bitfield) {
    const unsigned size = 2;
    bitvec<> bv(size), negbv(size), expNegbv(size);
    // bv = 01
    bv[1] = true;
    negbv = ~bv;

    // expNegbv = 10
    expNegbv[0] = true;

    EXPECT_EQ(expNegbv, negbv);
}

TEST(bitvec, can_invert_large_bitfield) {
    const unsigned size = 38;
    bitvec<> bv(size), negbv(size), expNegbv(size);
    bv[35] = true;
    negbv = ~bv;

    for(int i = 0; i < size; i++)
    expNegbv[i] = true;
    expNegbv[35] = false;

    EXPECT_EQ(expNegbv, negbv);
}

TEST(bitvec, can_invert_many_random_bits_bitfield) {
    const unsigned size = 38;
    bitvec<> bv(size), negbv(size), expNegbv(size);

    std::vector<int> bits;
    bits.push_back(0);
    bits.push_back(1);
    bits.push_back(14);
    bits.push_back(16);
    bits.push_back(33);
    bits.push_back(37);

    for (unsigned i = 0; i < bits.size(); i++) bv[bits[i]] = true;

    negbv = ~bv;

    for(unsigned i = 0; i < size; i++) expNegbv[i] = true;
    for (unsigned i = 0; i < bits.size(); i++) expNegbv[bits[i]] = false;

    EXPECT_EQ(expNegbv, negbv);
}

TEST(bitvec, bitfields_with_different_bits_are_not_equal) {
    const unsigned size = 4;
    bitvec<> bv1(size), bv2(size);

    bv1[1] = true;
    bv1[3] = true;

    bv2[1] = true;
    bv2[2] = true;

    EXPECT_NE(bv1, bv2);
}
// NOLINTEND
