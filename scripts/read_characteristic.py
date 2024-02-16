import asyncio
import sys
import time
import bleak

LUX_SERVICE_UUID = "e552241e-773c-1246-987d-8417d304edb5"
LUX_CHAR_UUID    = "e552241e-773c-1246-987d-8417d304edb6"

async def main(address):
    """Connect to device and read its lux Level characteristic."""
    try:
        async with bleak.BleakClient(address) as client:
            while(1):
                try:
                    lux_service = client.services.get_service(LUX_SERVICE_UUID)
                except AttributeError as err:
                    print("Unable to get service with UUID of {}".format(LUX_SERVICE_UUID))
                    return

                try:
                    lux_level_characteristic = (lux_service.get_characteristic(LUX_CHAR_UUID))
                    lux_level = await client.read_gatt_char(lux_level_characteristic)
                except AttributeError as err:
                    print("Unable to get characteristic with UUID of {}".format(LUX_CHAR_UUID))
                    print("Trying another approach...")
                    lux_level = await client.read_gatt_char(LUX_CHAR_UUID)
                    return

                print(f"Lux level: {int(lux_level[0])}")
                time.sleep(2)

    except asyncio.exceptions.TimeoutError:
        print(f"Can’t connect to device {address}.")


if __name__ == "__main__":
    if len(sys.argv) == 2:
        address = sys.argv[1]
        asyncio.run(main(address))
else:
    print("Please specify the Bluetooth address.")