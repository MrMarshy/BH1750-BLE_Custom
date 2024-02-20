import asyncio
import sys
import bleak

LUX_SERVICE_UUID = "e552241e-773c-1246-987d-8417d304edb5"
LUX_CHAR_UUID    = "e552241e-773c-1246-987d-8417d304edb6"

def lux_level_changed(handle: int, data: bytearray):
    """ Show the lux level """
    print("Lux Level: {}".format(int.from_bytes(data, byteorder='little', signed=True)))

async def main(address):
    """Connect to device and subscribe to its Lux Level notificatins."""
    try:
        async with bleak.BleakClient(address) as client:
            print("Connected to address: {}".format(address))
            await client.start_notify(LUX_CHAR_UUID, lux_level_changed)
            print("Notifications started...")

            while(1):
                await asyncio.sleep(1)

    except asyncio.exceptions.TimeoutError:
        print(f"Can’t connect to device {address}.")


if __name__ == "__main__":
    if len(sys.argv) == 2:
        address = sys.argv[1]
        asyncio.run(main(address))
else:
    print("Please specify the Bluetooth address.")
