#include <stdio.h>

int ixland_linux_compat_test_abi(void);
int ixland_linux_compat_test_host_boundary(void);

int main(void) {
    int failures = 0;

    failures += ixland_linux_compat_test_abi();
    failures += ixland_linux_compat_test_host_boundary();

    if (failures == 0) {
        printf("ixland linux compatibility tests passed\n");
        return 0;
    }

    printf("ixland linux compatibility tests failed: %d\n", failures);
    return 1;
}
