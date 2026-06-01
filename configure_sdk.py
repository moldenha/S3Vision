# This is a file dedicated to configuring the sdkconfig 
# You should also configure the main/main.cpp slightly for your configuration and know a little c++

import argparse


parser = argparse.ArgumentParser()
parser.add_argument("-b", "--battery", action='store_true')
parser.add_argument("-d", "--debug", action='store_true')
parser.add_argument("-c", "--console", action='store_true')
args = parser.parse_args()

options = [args.battery, args.debug, args.console]
if True not in options:
    raise Exception("Error: must choose battery, debug, or console")
if options.count(True) > 1:
    raise Exception("Error: must only select battery, debug, or console")


option = ['b', 'd', 'c'][options.index(True)]


# Key: (the line containing this to be changed
#   - d: debug
#   - c: console (not debug but if something goes wrong)
#   - b: battery (when being run on battery life)
configs = {
        'CONFIG_USJ_ENABLE_USB_SERIAL_JTAG' :
        {
            'd' : 'CONFIG_USJ_ENABLE_USB_SERIAL_JTAG=y',
            'c' : 'CONFIG_USJ_ENABLE_USB_SERIAL_JTAG=y',
            'b' : 'CONFIG_USJ_ENABLE_USB_SERIAL_JTAG=n' # <- doing this will save 5-10 mA of constant draining
        },
        'CONFIG_ESP_ROM_CONSOLE_OUTPUT_SECONDARY' :
        {
            'd' : 'CONFIG_ESP_ROM_CONSOLE_OUTPUT_SECONDARY=y',
            'c' : 'CONFIG_ESP_ROM_CONSOLE_OUTPUT_SECONDARY=n', # <- Disabling saves speed (and a tiny fraction of power) by preventing the CPU from processing and pushing secondary UART data to the debug pins
            'b' : 'CONFIG_ESP_ROM_CONSOLE_OUTPUT_SECONDARY=n'
        },
        'CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG' :
        {
            'd' : 'CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y',
            'c' : 'CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y',
            'b' : 'CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=n'  # <- Saves power by preventing clocks from running during sleep modes pre-created during code running
        },
        'CONFIG_ESP_CONSOLE_SECONDARY_NONE' : 
        {
            'd' : 'CONFIG_ESP_CONSOLE_SECONDARY_NONE=y',
            'c' : 'CONFIG_ESP_CONSOLE_SECONDARY_NONE=n',
            'b' : 'CONFIG_ESP_CONSOLE_SECONDARY_NONE=n' # <- Saves iRAM and DRAM preventing cache misses from hotpath (saving speed and power)
        },
        'CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG_ENABLED' :
        {
            'd' : 'CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG_ENABLED=y',
            'c' : 'CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG_ENABLED=y',
            'b' : 'CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG_ENABLED=n' # <- Saves power by allowing APB clock to be turned off during run
        },
        'CONFIG_BOOTLOADER_LOG_LEVEL_NONE' :
        {
            'd' : '# CONFIG_BOOTLOADER_LOG_LEVEL_NONE is not set',
            'c' : 'CONFIG_BOOTLOADER_LOG_LEVEL_NONE=y',
            'b' : 'CONFIG_BOOTLOADER_LOG_LEVEL_NONE=y'
        },
        'CONFIG_BOOTLOADER_LOG_LEVEL_INFO' :
        {
            'd' : 'CONFIG_BOOTLOADER_LOG_LEVEL_INFO=y',
            'c' : '# CONFIG_BOOTLOADER_LOG_LEVEL_INFO is not set',
            'b' : '# CONFIG_BOOTLOADER_LOG_LEVEL_INFO is not set'
        },
        'CONFIG_BOOTLOADER_LOG_LEVEL' :
        {
            'd' : 'CONFIG_BOOTLOADER_LOG_LEVEL=3',
            'c' : 'CONFIG_BOOTLOADER_LOG_LEVEL=0',
            'b' : 'CONFIG_BOOTLOADER_LOG_LEVEL=0'
        },
        # Power Management: Enable DFS + tickless idle for battery
        'CONFIG_PM_ENABLE' :
        {
            'd' : 'CONFIG_PM_ENABLE=y',
            'c' : 'CONFIG_PM_ENABLE=y',
            'b' : 'CONFIG_PM_ENABLE=y'  # Already set, but make explicit
        },
        'CONFIG_PM_DFS_INIT_AUTO' :
        {
            'd' : '# CONFIG_PM_DFS_INIT_AUTO is not set',
            'c' : '# CONFIG_PM_DFS_INIT_AUTO is not set',
            'b' : '# CONFIG_PM_DFS_INIT_AUTO is not set'  # Manual control via esp_pm_configure()
        },
        'CONFIG_FREERTOS_USE_TICKLESS_IDLE' :
        {
            'd' : '# CONFIG_FREERTOS_USE_TICKLESS_IDLE is not set',
            'c' : '# CONFIG_FREERTOS_USE_TICKLESS_IDLE is not set',
            'b' : '# CONFIG_FREERTOS_USE_TICKLESS_IDLE is not set' # setting this to y breaks camera timing and allocation behavior
        },
        # WiFi: Fully disable for battery
        'CONFIG_ESP_WIFI_ENABLED' :
        {
            'd' : 'CONFIG_ESP_WIFI_ENABLED=y',
            'c' : 'CONFIG_ESP_WIFI_ENABLED=n',
            'b' : 'CONFIG_ESP_WIFI_ENABLED=n'
        },
        'CONFIG_ESP_WIFI_IRAM_OPT' :
        {
            'd' : 'CONFIG_ESP_WIFI_IRAM_OPT=n',
            'c' : 'CONFIG_ESP_WIFI_IRAM_OPT=n',
            'b' : 'CONFIG_ESP_WIFI_IRAM_OPT=n'
        },
        'CONFIG_ESP_WIFI_RX_IRAM_OPT' :
        {
            'd' : 'CONFIG_ESP_WIFI_RX_IRAM_OPT=n',
            'c' : 'CONFIG_ESP_WIFI_RX_IRAM_OPT=n',
            'b' : 'CONFIG_ESP_WIFI_RX_IRAM_OPT=n'
        },
        'CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM' :
        {
            'd' : 'CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM=2',
            'c' : 'CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM=2',
            'b' : 'CONFIG_ESP_WIFI_STATIC_RX_BUFFER_NUM=2'
        },
        'CONFIG_ESP_WIFI_DYNAMIC_RX_BUFFER_NUM' :
        {
            'd' : 'CONFIG_ESP_WIFI_DYNAMIC_RX_BUFFER_NUM=4',
            'c' : 'CONFIG_ESP_WIFI_DYNAMIC_RX_BUFFER_NUM=4',
            'b' : 'CONFIG_ESP_WIFI_DYNAMIC_RX_BUFFER_NUM=4'
        },
        # LwIP: Disable (no networking at all_
        'CONFIG_ESP_NETIF_ENABLED' :
        {
            'd' : 'CONFIG_ESP_NETIF_ENABLED=n',
            'c' : 'CONFIG_ESP_NETIF_ENABLED=n',
            'b' : 'CONFIG_ESP_NETIF_ENABLED=n'
        },
        'CONFIG_LWIP_ENABLED' :
        {
            'd' : 'CONFIG_LWIP_ENABLED=n',
            'c' : 'CONFIG_LWIP_ENABLED=n',
            'b' : 'CONFIG_LWIP_ENABLED=n'
        },
        # Heap: Disable poisoning overhead in battery mode (detects memory corruption)
        'CONFIG_HEAP_POISONING_DISABLED':
        {
            'd' : '# CONFIG_HEAP_POISONING_DISABLED is not set',
            'c' : '# CONFIG_HEAP_POISONING_DISABLED is not set',
            'b' : 'CONFIG_HEAP_POISONING_DISABLED=y'
        },
        'CONFIG_HEAP_POISONING_LIGHT' :
        {
            'd' : 'CONFIG_HEAP_POISONING_LIGHT=y',
            'c' : 'CONFIG_HEAP_POISONING_LIGHT=y',
            'b' : '# CONFIG_HEAP_POISONING_LIGHT is not set'
        },
        'CONFIG_HEAP_POISONING_COMPREHENSIVE' :
        {
            'd' : '# CONFIG_HEAP_POISONING_COMPREHENSIVE is not set',
            'c' : '# CONFIG_HEAP_POISONING_COMPREHENSIVE is not set',
            'b' : '# CONFIG_HEAP_POISONING_COMPREHENSIVE is not set'
        },
        # Bluetooth: Fully disable (you don't use it)
        'CONFIG_BT_ENABLED' :
        {
            'd' : '# CONFIG_BT_ENABLED is not set',
            'c' : '# CONFIG_BT_ENABLED is not set',
            'b' : '# CONFIG_BT_ENABLED is not set'
        },

        # PSRAM: Optimize for battery (already mostly good, but be explicit)
        #   It is actually vital for accuracy and timing that this is 40M - will break program if not
        'CONFIG_SPIRAM_SPEED_40M' :
        {
            'd' : 'CONFIG_SPIRAM_SPEED_40M=y',
            'c' : 'CONFIG_SPIRAM_SPEED_40M=y',
            'b' : 'CONFIG_SPIRAM_SPEED_40M=y'  # Lower speed = lower power than 80M
        },
        'CONFIG_SPIRAM_SPEED_80M' :
        {
            'd' : '# CONFIG_SPIRAM_SPEED_80M is not set',
            'c' : '# CONFIG_SPIRAM_SPEED_80M is not set',
            'b' : '# CONFIG_SPIRAM_SPEED_80M is not set'
        },

        # IRAM/DRAM optimization
        'CONFIG_ESP_WIFI_IRAM_OPT' :
        {
            'd' : 'CONFIG_ESP_WIFI_IRAM_OPT=n',
            'c' : 'CONFIG_ESP_WIFI_IRAM_OPT=n',
            'b' : 'CONFIG_ESP_WIFI_IRAM_OPT=n'
        },
        'CONFIG_SPIRAM_FETCH_INSTRUCTIONS' :
        {
            'd' : 'CONFIG_SPIRAM_FETCH_INSTRUCTIONS=y',
            'c' : 'CONFIG_SPIRAM_FETCH_INSTRUCTIONS=y',
            'b' : 'CONFIG_SPIRAM_FETCH_INSTRUCTIONS=y'  # Keep this for battery (reduces cache misses)
        },
        'CONFIG_SPIRAM_RODATA' :
        {
            'd' : 'CONFIG_SPIRAM_RODATA=y',
            'c' : 'CONFIG_SPIRAM_RODATA=y',
            'b' : 'CONFIG_SPIRAM_RODATA=y'  # Keep this too
        },
        # Reduce stack overhead
        'CONFIG_ESP_TASK_WDT_INIT_STACK_SIZE' :
        {
            'd' : 'CONFIG_ESP_TASK_WDT_INIT_STACK_SIZE=4096',
            'c' : 'CONFIG_ESP_TASK_WDT_INIT_STACK_SIZE=4096',
            'b' : 'CONFIG_ESP_TASK_WDT_INIT_STACK_SIZE=4096'
        },
        
        # NVS: Only needed for WiFi/BT
        'CONFIG_NVS_ENCRYPTION_ENABLED' :
        {
            'd' : 'CONFIG_NVS_ENCRYPTION_ENABLED=n',
            'c' : 'CONFIG_NVS_ENCRYPTION_ENABLED=n',
            'b' : 'CONFIG_NVS_ENCRYPTION_ENABLED=n'
        },
}


config_keys = list(configs.keys())

def get_file():
    with open("sdkconfig", 'r') as f:
        return f.readlines()

def write_file(lines):
    with open("sdkconfig", 'w') as f:
        return f.writelines(lines)

def parse_line(line):
    for key in config_keys:
        if key in line:
            return configs[key][option] + '\n'
    return line

def parse_lines(lines):
    return [parse_line(line) for line in lines]

if __name__ == '__main__':
    write_file(parse_lines(get_file()))
