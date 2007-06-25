#include "TestPrologue.h"

typedef lrucache<char, int> test_lrucache;

TEST(lrucache_starts_empty) {
    test_lrucache c(2);
    CHECK_EQUAL(0, c.size());
}

TEST(lrucache_empty_returns_false_isset) {
    test_lrucache c(2);
    CHECK_EQUAL(false, c.isset('a'));
}

TEST(lrucache_insert_1) {
    test_lrucache c(2);
    c.set('a', 42);
    CHECK_EQUAL(1, c.size());
    CHECK(c.get('a'));
    CHECK_EQUAL(42, *c.get('a'));
}

TEST(lrucache_insert_2) {
    test_lrucache c(2);
    c.set('a', 42);
    c.set('b', 52);
    CHECK_EQUAL(2, c.size());
    CHECK_EQUAL(true, c.isset('a'));
    CHECK_EQUAL(true, c.isset('b'));
}

TEST(lrucache_expire_oldest_when_max_size_exceeded) {
    test_lrucache c(2);
    c.set('a', 42);
    c.set('b', 52);
    c.set('c', 33);
    CHECK_EQUAL(2, c.size());
    CHECK_EQUAL(false, c.isset('a'));
    CHECK_EQUAL(true, c.isset('b'));
    CHECK_EQUAL(true, c.isset('c'));
}

TEST(lrucache_resetting_key_changes_age) {
    test_lrucache c(2);
    c.set('a', 42);
    c.set('b', 52);
    c.set('a', 42);
    c.set('c', 33);
    CHECK_EQUAL(2, c.size());
    CHECK_EQUAL(true, c.isset('a'));
    CHECK_EQUAL(false, c.isset('b'));
    CHECK_EQUAL(true, c.isset('c'));
}

TEST(lrucache_getting_key_changes_age) {
    test_lrucache c(2);
    c.set('a', 42);
    c.set('b', 52);
    c.get('a');
    c.set('c', 33);
    CHECK_EQUAL(2, c.size());
    CHECK_EQUAL(true, c.isset('a'));
    CHECK_EQUAL(false, c.isset('b'));
    CHECK_EQUAL(true, c.isset('c'));
}

TEST(lrucache_many_gets_sets) {
    test_lrucache c(23);
    for(int i=0; i<1000; i++) {
        c.set('a'+i, i);
    }
    for(int i=0; i<1000; i++) {
        c.get('a'+i);
    }
    for(int i=0; i<1000; i++) {
        c.get('a'+i);
        c.set('a'+i, 200+i);
    }
}
