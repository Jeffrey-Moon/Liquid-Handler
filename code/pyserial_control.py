import serial
import time
import argparse


def coordinate_pair(s):
    try:
        x, y = map(int, s.split(','))
        return (x, y)
    except:
        raise argparse.ArgumentTypeError("Coordinates must be x,y")


parser = argparse.ArgumentParser(description='Control liquid handler via Arduino')
parser.add_argument('--repetitions', type=int, nargs='+', required=True, help='List of repetition counts')
parser.add_argument('--from_coords', type=coordinate_pair, nargs='+', required=True, help='List of "from" coordinates in format x,y')
parser.add_argument('--to_coords', type=coordinate_pair, nargs='+', required=True, help='List of "to" coordinates in format x,y')
parser.add_argument('--tube_types', type=str, nargs='+', required=True, help='List of tube types (B or T)')


def wait_for_arduino(arduino):
    # Wait for Arduino to send "Arduino ready"
    while True:
        if arduino.in_waiting:
            response = arduino.readline().decode().strip()
            print(f"Arduino says: {response}")
            if response == "Arduino ready":
                break
        time.sleep(0.1)


def send_command(arduino, command):
    start_time = time.time()  # Record start time
    print(f"Sending: {command}")
    arduino.write(command.encode() + b'\n')

    # Wait for command to complete
    while True:
        if arduino.in_waiting:
            response = arduino.readline().decode().strip()
            print(f"Received: {response}")
            if response == "Command completed":
                end_time = time.time()  # Record end time
                elapsed_time = end_time - start_time
                print(f"Time taken for command '{command}': {elapsed_time:.2f} seconds")
                break
        time.sleep(0.1)


def execute_sequence(arduino, repetitions, from_coords, to_coords, tube_types):
    sequence = []
    for rep, from_coord, to_coord, tube_type in zip(repetitions, from_coords, to_coords, tube_types):
        sequence.append(f"T")  # Change tip
        for _ in range(rep):
            if tube_type == 'B':
                sequence.append(f"B2T,{from_coord[0]},{from_coord[1]},{to_coord[0]},{to_coord[1]}")
            elif tube_type == 'T':
                sequence.append(f"T2T,{from_coord[0]},{from_coord[1]},{to_coord[0]},{to_coord[1]}")
    sequence.append("R")  # Reset

    for command in sequence:
        send_command(arduino, command)


if __name__ == '__main__':
    args = parser.parse_args()
    
    arduino = serial.Serial(port='COM3', baudrate=9600, timeout=1)
    time.sleep(2)  # Wait for Arduino to reset

    wait_for_arduino(arduino)  # Wait for Arduino to be ready

    execute_sequence(arduino, args.repetitions, args.from_coords, args.to_coords, args.tube_types)

    arduino.close()