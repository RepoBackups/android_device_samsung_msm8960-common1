/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (c) 2012 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOG_TAG "CM PowerHAL"
#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>

#define UNUSED __attribute__((unused))

#define SCALING_GOVERNOR_PATH "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"

static char governor[20];

static int sysfs_read(char *path, char *s, int num_bytes)
{
    char buf[80];
    int count;
    int ret = 0;
    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);

        return -1;
    }

    if ((count = read(fd, s, num_bytes - 1)) < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);

        ret = -1;
    } else {
        s[count] = '\0';
    }

    close(fd);

    return ret;
}

static void sysfs_write(char *path, char *s)
{
    char buf[80];
    int len;
    int fd = open(path, O_WRONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);
    }

    close(fd);
}

static int get_scaling_governor() {
    if (sysfs_read(SCALING_GOVERNOR_PATH, governor,
                sizeof(governor)) == -1) {
        return -1;
    } else {
        // Strip newline at the end.
        int len = strlen(governor);

        len--;

        while (len >= 0 && (governor[len] == '\n' || governor[len] == '\r'))
            governor[len--] = '\0';
    }

    return 0;
}

static void configure_governor()
{
    if (strncmp(governor, "badass", 6) == 0) {
        sysfs_write("/sys/devices/system/cpu/cpufreq/badass/up_threshold", "90");
        sysfs_write("/sys/devices/system/cpu/cpufreq/badass/io_is_busy", "1");
        sysfs_write("/sys/devices/system/cpu/cpufreq/badass/sampling_down_factor", "4");
        sysfs_write("/sys/devices/system/cpu/cpufreq/badass/down_differential", "10");
        sysfs_write("/sys/devices/system/cpu/cpufreq/badass/up_threshold_multi_core", "70");
        sysfs_write("/sys/devices/system/cpu/cpufreq/badass/down_differential_multi_core", "3");
        sysfs_write("/sys/devices/system/cpu/cpufreq/badass/optimal_freq", "918000");
        sysfs_write("/sys/devices/system/cpu/cpufreq/badass/sync_freq", "1026000");
        sysfs_write("/sys/devices/system/cpu/cpufreq/badass/up_threshold_any_cpu_load", "80");
        sysfs_write("/sys/devices/system/cpu/cpufreq/badass/input_boost", "1026000");
        sysfs_write("/sys/devices/system/cpu/cpufreq/badass/sampling_rate", "50000");
    } else if (strncmp(governor, "ondemand", 8) == 0) {
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/up_threshold", "90");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/io_is_busy", "1");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor", "4");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/down_differential", "10");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/up_threshold_multi_core", "70");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/down_differential_multi_core", "3");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/optimal_freq", "918000");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/sync_freq", "1026000");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/up_threshold_any_cpu_load", "80");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/input_boost", "1026000");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/sampling_rate", "50000");
    } else if (strncmp(governor, "interactive", 11) == 0) {
        sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/min_sample_time", "40000");
        sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/hispeed_freq", "918000");
        sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/above_hispeed_delay", "90000");
        sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/go_hispeed_load", "90");
        sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/timer_rate", "30000");
        sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/io_is_busy", "1");
    } else if (strncmp(governor, "uberdemand", 10) == 0) {
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/up_threshold", "90");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/io_is_busy", "1");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor", "4");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/down_differential", "10");
        sysfs_write("/sys/devices/system/cpu/cpufreq/ondemand/sampling_rate", "50000");
    }
}

static void cm_power_set_interactive(UNUSED struct power_module *module, UNUSED int on)
{
    get_scaling_governor();
    configure_governor();
}

static int boostpulse_open(UNUSED struct power_module *module)
{
    get_scaling_governor();
    configure_governor();
    return 0;
}

static void cm_power_hint(UNUSED struct power_module *module, power_hint_t hint,
                            UNUSED void *data)
{
    get_scaling_governor();
    configure_governor();
}

static void cm_power_init(UNUSED struct power_module *module)
{
    get_scaling_governor();
    configure_governor();
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_2,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "CM MSM8960 Power HAL",
        .author = "The CyanogenMod Project",
        .methods = &power_module_methods,
    },
    .init = cm_power_init,
    .powerHint = cm_power_hint,
    .setInteractive = cm_power_set_interactive,
};
