import asyncio
import sys

from bleak import BleakClient

async def main(address):
    """ Connect to a BLE device and show its services."""
    async with BleakClient(address) as client:
        for service in client.services:
            print("- Description: {}".format(service.description))
            print("- UUID: {}".format(service.uuid))
            print("- Handle: {}".format(service.handle))

            for char in service.characteristics:
                value = None
                if "read" in char.properties:
                    try:
                        value = bytes(await client.read_gatt_char(char))
                    except Exception as error:
                            value = error
                
                print("- Description: {}".format(char.description))
                print("- UUID: {}".format(char.uuid))
                print("- Handle: {}".format(char.handle))
                print("- Properties: {}".format(", ".join(char.properties)))
                print("- Value: {}", value)

                for descriptor in char.descriptors:
                    desc_value = None
                    try:
                        desc_value = bytes(await client.read_gatt_descriptor(descriptor))
                    except Exception as error:
                        desc_value = error
                    
                    print("- UUID: {}".format(descriptor.uuid))
                    print("- Handle: {}".format(descriptor.handle))
                    print("- Value: {}".format(value))

if __name__ == "__main__":
    if len(sys.argv) == 2:
        address = sys.argv[1]
        asyncio.run(main(address))
    else:
        print("Please specify the Bluetooth address.")