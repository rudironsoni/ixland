/* Unit tests for identity syscalls
 * Tests: iox_getuid, iox_geteuid, iox_setuid, iox_getgid, iox_getegid, iox_setgid
 */

#include <errno.h>
#include <stdbool.h>
#include <unistd.h>

#include "../../src/iox/internal/iox_internal.h"
#include "../harness/iox_test.h"

IOX_TEST(getuid_returns_consistent_uid) {
    uid_t uid1 = iox_getuid();
    uid_t uid2 = iox_getuid();
    uid_t uid3 = iox_getuid();

    /* All calls should return the same UID */
    IOX_ASSERT_EQ(uid1, uid2);
    IOX_ASSERT_EQ(uid2, uid3);
    IOX_ASSERT_GT(uid1, 0);

    return true;
}

IOX_TEST(getuid_returns_ios_default) {
    uid_t uid = iox_getuid();

    /* On iOS, default UID is typically 501 (mobile user) */
    /* We allow either 0 (root) or 501 (mobile) as valid iOS UIDs */
    IOX_ASSERT(uid == 0 || uid == 501);

    return true;
}

IOX_TEST(geteuid_returns_consistent_euid) {
    uid_t euid1 = iox_geteuid();
    uid_t euid2 = iox_geteuid();

    /* All calls should return the same EUID */
    IOX_ASSERT_EQ(euid1, euid2);
    IOX_ASSERT_GT(euid1, 0);

    return true;
}

IOX_TEST(geteuid_matches_getuid_on_ios) {
    uid_t uid = iox_getuid();
    uid_t euid = iox_geteuid();

    /* On iOS, effective UID should match real UID */
    IOX_ASSERT_EQ(uid, euid);

    return true;
}

IOX_TEST(setuid_returns_eperm) {
    /* On iOS, setuid should always fail with EPERM */
    int result = iox_setuid(0);

    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EPERM);

    /* Try with another UID */
    errno = 0;
    result = iox_setuid(501);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EPERM);

    return true;
}

IOX_TEST(getgid_returns_consistent_gid) {
    gid_t gid1 = iox_getgid();
    gid_t gid2 = iox_getgid();
    gid_t gid3 = iox_getgid();

    /* All calls should return the same GID */
    IOX_ASSERT_EQ(gid1, gid2);
    IOX_ASSERT_EQ(gid2, gid3);
    IOX_ASSERT_GT(gid1, 0);

    return true;
}

IOX_TEST(getgid_returns_ios_default) {
    gid_t gid = iox_getgid();

    /* On iOS, default GID is typically 501 (mobile group) */
    /* We allow either 0 (wheel) or 501 (mobile) as valid iOS GIDs */
    IOX_ASSERT(gid == 0 || gid == 501 || gid == 20);

    return true;
}

IOX_TEST(getegid_returns_consistent_egid) {
    gid_t egid1 = iox_getegid();
    gid_t egid2 = iox_getegid();

    /* All calls should return the same EGID */
    IOX_ASSERT_EQ(egid1, egid2);
    IOX_ASSERT_GT(egid1, 0);

    return true;
}

IOX_TEST(getegid_matches_getgid_on_ios) {
    gid_t gid = iox_getgid();
    gid_t egid = iox_getegid();

    /* On iOS, effective GID should match real GID */
    IOX_ASSERT_EQ(gid, egid);

    return true;
}

IOX_TEST(setgid_returns_eperm) {
    /* On iOS, setgid should always fail with EPERM */
    int result = iox_setgid(0);

    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EPERM);

    /* Try with another GID */
    errno = 0;
    result = iox_setgid(501);
    IOX_ASSERT_EQ(result, -1);
    IOX_ASSERT_EQ(errno, EPERM);

    return true;
}

IOX_TEST(identity_values_consistent_across_calls) {
    /* Verify that multiple identity syscalls return consistent values */
    uid_t uid1 = iox_getuid();
    gid_t gid1 = iox_getgid();

    for (int i = 0; i < 100; i++) {
        uid_t uid2 = iox_getuid();
        gid_t gid2 = iox_getgid();

        IOX_ASSERT_EQ(uid1, uid2);
        IOX_ASSERT_EQ(gid1, gid2);
    }

    return true;
}

IOX_TEST(identity_syscalls_no_side_effects) {
    /* Calling get functions should not affect errno */
    errno = 0;

    iox_getuid();
    IOX_ASSERT_EQ(errno, 0);

    iox_geteuid();
    IOX_ASSERT_EQ(errno, 0);

    iox_getgid();
    IOX_ASSERT_EQ(errno, 0);

    iox_getegid();
    IOX_ASSERT_EQ(errno, 0);

    return true;
}
