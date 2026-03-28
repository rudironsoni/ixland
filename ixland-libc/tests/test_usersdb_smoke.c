/* ixland-libc-usersdb smoke tests
 *
 * Minimal compile-check and basic validation for ixland-libc-usersdb.
 */

#include <grp.h>
#include <pwd.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

/* ============================================================================
 * GROUP DATABASE TESTS
 * ============================================================================ */

static int test_getgrnam(void) {
    struct group *grp;

    /* Test known group (mobile) */
    grp = getgrnam("mobile");
    assert(grp != NULL);
    assert(strcmp(grp->gr_name, "mobile") == 0);
    assert(grp->gr_gid == 501);
    printf("  getgrnam(\"mobile\"): name=%s, gid=%d\n", grp->gr_name, grp->gr_gid);

    /* Test unknown group */
    grp = getgrnam("nonexistent");
    assert(grp == NULL);
    printf("  getgrnam(\"nonexistent\"): NULL (expected)\n");

    /* Test NULL input */
    errno = 0;
    grp = getgrnam(NULL);
    assert(grp == NULL);
    assert(errno == EINVAL);
    printf("  getgrnam(NULL): NULL, errno=EINVAL (expected)\n");

    return 0;
}

static int test_getgrgid(void) {
    struct group *grp;

    /* Test known GID (501 = mobile) */
    grp = getgrgid(501);
    assert(grp != NULL);
    assert(grp->gr_gid == 501);
    printf("  getgrgid(501): name=%s, gid=%d\n", grp->gr_name, grp->gr_gid);

    /* Test unknown GID */
    grp = getgrgid(999);
    assert(grp == NULL);
    printf("  getgrgid(999): NULL (expected)\n");

    return 0;
}

static int test_getgrnam_r(void) {
    struct group grp;
    char buf[256];
    struct group *result;
    int ret;

    /* Test known group */
    ret = getgrnam_r("mobile", &grp, buf, sizeof(buf), &result);
    assert(ret == 0);
    assert(result != NULL);
    assert(result->gr_gid == 501);
    printf("  getgrnam_r(\"mobile\"): gid=%d\n", result->gr_gid);

    /* Test unknown group */
    ret = getgrnam_r("nonexistent", &grp, buf, sizeof(buf), &result);
    assert(ret == ENOENT);
    assert(result == NULL);
    printf("  getgrnam_r(\"nonexistent\"): ENOENT (expected)\n");

    /* Test buffer too small */
    ret = getgrnam_r("mobile", &grp, buf, 1, &result);
    assert(ret == ERANGE);
    assert(result == NULL);
    printf("  getgrnam_r(\"mobile\", small buf): ERANGE (expected)\n");

    return 0;
}

static int test_getgroups(void) {
    gid_t groups[16];
    int ngroups;

    /* Test getting number of groups */
    ngroups = getgroups(0, NULL);
    assert(ngroups >= 0);
    printf("  getgroups(0, NULL): %d\n", ngroups);

    /* Test getting actual groups */
    ngroups = getgroups(16, groups);
    assert(ngroups >= 0);
    if (ngroups > 0) {
        printf("  getgroups(16, groups): %d groups, first gid=%d\n", ngroups, groups[0]);
    } else {
        printf("  getgroups(16, groups): %d groups\n", ngroups);
    }

    /* Test invalid size */
    errno = 0;
    ngroups = getgroups(-1, groups);
    assert(ngroups == -1);
    assert(errno == EINVAL);
    printf("  getgroups(-1, groups): -1, errno=EINVAL (expected)\n");

    return 0;
}

/* ============================================================================
 * PASSWORD DATABASE TESTS
 * ============================================================================ */

static int test_getpwnam(void) {
    struct passwd *pwd;

    /* Test known user (mobile) */
    pwd = getpwnam("mobile");
    assert(pwd != NULL);
    assert(strcmp(pwd->pw_name, "mobile") == 0);
    assert(pwd->pw_uid == 501);
    assert(pwd->pw_gid == 501);
    printf("  getpwnam(\"mobile\"): name=%s, uid=%d, gid=%d\n",
           pwd->pw_name, pwd->pw_uid, pwd->pw_gid);

    /* Test root user */
    pwd = getpwnam("root");
    assert(pwd != NULL);
    assert(pwd->pw_uid == 0);
    printf("  getpwnam(\"root\"): uid=%d\n", pwd->pw_uid);

    /* Test unknown user */
    pwd = getpwnam("nonexistent");
    assert(pwd == NULL);
    printf("  getpwnam(\"nonexistent\"): NULL (expected)\n");

    /* Test NULL input */
    errno = 0;
    pwd = getpwnam(NULL);
    assert(pwd == NULL);
    assert(errno == EINVAL);
    printf("  getpwnam(NULL): NULL, errno=EINVAL (expected)\n");

    return 0;
}

static int test_getpwuid(void) {
    struct passwd *pwd;

    /* Test known UID (501 = mobile) */
    pwd = getpwuid(501);
    assert(pwd != NULL);
    assert(pwd->pw_uid == 501);
    printf("  getpwuid(501): name=%s, uid=%d\n", pwd->pw_name, pwd->pw_uid);

    /* Test root UID (0) */
    pwd = getpwuid(0);
    assert(pwd != NULL);
    assert(pwd->pw_uid == 0);
    printf("  getpwuid(0): name=%s, uid=%d\n", pwd->pw_name, pwd->pw_uid);

    /* Test unknown UID */
    pwd = getpwuid(999);
    assert(pwd == NULL);
    printf("  getpwuid(999): NULL (expected)\n");

    return 0;
}

static int test_getpwnam_r(void) {
    struct passwd pwd;
    char buf[512];
    struct passwd *result;
    int ret;

    /* Test known user */
    ret = getpwnam_r("mobile", &pwd, buf, sizeof(buf), &result);
    assert(ret == 0);
    assert(result != NULL);
    assert(result->pw_uid == 501);
    printf("  getpwnam_r(\"mobile\"): uid=%d\n", result->pw_uid);

    /* Test unknown user */
    ret = getpwnam_r("nonexistent", &pwd, buf, sizeof(buf), &result);
    assert(ret == ENOENT);
    assert(result == NULL);
    printf("  getpwnam_r(\"nonexistent\"): ENOENT (expected)\n");

    /* Test buffer too small */
    ret = getpwnam_r("mobile", &pwd, buf, 1, &result);
    assert(ret == ERANGE);
    assert(result == NULL);
    printf("  getpwnam_r(\"mobile\", small buf): ERANGE (expected)\n");

    return 0;
}

static int test_passwd_iteration(void) {
    struct passwd *pwd;
    int count = 0;

    setpwent();
    printf("  setpwent() / getpwent() iteration:\n");

    while ((pwd = getpwent()) != NULL) {
        printf("    entry %d: %s (uid=%d)\n", count, pwd->pw_name, pwd->pw_uid);
        count++;
        if (count > 10) {
            printf("    (stopping at 10 entries)\n");
            break;
        }
    }

    endpwent();
    printf("  endpwent(): iterated %d entries\n", count);

    assert(count >= 2);  /* Should have at least root and mobile */
    return 0;
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(void) {
    int result = 0;

    printf("ixland-libc-usersdb smoke tests\n");
    printf("================================\n\n");

    printf("Group Database Tests:\n");
    result |= test_getgrnam();
    result |= test_getgrgid();
    result |= test_getgrnam_r();
    result |= test_getgroups();
    printf("\n");

    printf("Password Database Tests:\n");
    result |= test_getpwnam();
    result |= test_getpwuid();
    result |= test_getpwnam_r();
    result |= test_passwd_iteration();
    printf("\n");

    if (result == 0) {
        printf("All tests PASSED\n");
    } else {
        printf("Some tests FAILED\n");
    }

    return result;
}
