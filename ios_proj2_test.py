#!/usr/bin/env python3
# IOS 2024/25 proj2 test
# autor: @ondar.irl
# verze: 2

import argparse, subprocess, sys
from enum import Enum
from colorama import Style, Fore

DEFAULT_OUT = "./proj2.out"


## tridy pro procesy
class VehicleState(Enum):
    STARTED = 0
    WAITING = 1
    BOARDED = 2
    LEFT = 3


class FerryState(Enum):
    UNKNOWN = -1
    STARTED = 0
    ARRIVED_UNLOADING = 1
    ARRIVED_BOARDING = 2
    EN_ROUTE = 3
    FINISHED = 4


class Vehicle:
    def __init__(self, id):
        self.id = id
        self.port = "Unknown"
        self.state = VehicleState.STARTED

    def __str__(self):
        return f"{self.__class__.__name__} {self.id}, port {self.port}, state {self.state}"


class Car(Vehicle):
    count = 0
    def __init__(self, id):
        super().__init__(id)
        Car.count += 1


class Truck(Vehicle):
    count = 0
    def __init__(self, id):
        super().__init__(id)
        Truck.count += 1


class Ferry:
    def __init__(self):
        self.port = "Dock"
        self.state = FerryState.UNKNOWN
        self.loaded = {"N": 0, "O": 0}
        self.measured_capacity = 0
        self.trips = 0

    def __str__(self):
        return f"Ferry:\n\tLast visited port: {self.port}\n\tState: {self.state}\n\tTrucks on board: {self.loaded['N']}\n\tCars on board: {self.loaded['O']}\n\tMeasured capacity: {self.measured_capacity}\n\tTrips taken: {self.trips}"


## spusteni projektu a nasledna analyza
def run(exe: str, N: int, O: int, K: int, TC: int, TP: int) -> None:
    print(f"{Style.BRIGHT}Running {Style.NORMAL}'{exe} {N} {O} {K} {TC} {TP}'{Style.RESET_ALL}", flush=True)
    try:
        subprocess.run([exe, N, O, K, TC, TP], check=True, timeout=15)
    except subprocess.CalledProcessError as e:
        sys.exit(e.returncode)
    except subprocess.TimeoutExpired as e:
        print(f"{Style.BRIGHT}{Fore.RED}Timeout:{Style.NORMAL} {e}{Style.RESET_ALL}")
        sys.exit(1)
    except FileNotFoundError as e:
        print(f"{Style.BRIGHT}{Fore.RED}Executable not found:{Style.NORMAL} {e}{Style.RESET_ALL}")
        sys.exit(1)


## test logu
def analyze(path=DEFAULT_OUT, N: int = -1, O: int = -1, K: int = -1) -> None:
    print(f"{Style.BRIGHT}Analyzing file {Style.NORMAL}'{path}'{Style.RESET_ALL}", end="")
    if (N != -1 and O != -1 or K != -1):
        params = [f"{k}={v}" for k, v in (("N", N), ("O", O), ("K", K)) if v != -1]
        print(f", {Style.BRIGHT}checking against {Style.NORMAL}{', '.join(params)}{Style.RESET_ALL}")
    else:
        print(f", {Style.DIM}skipping ferry capacity and vehicle amount checks{Style.RESET_ALL}")
    with open(path, mode="r", encoding="utf-8") as f:
        lines = f.readlines()

    ferry = Ferry()
    vehicles: dict[dict[Vehicle]] = {
        "N": {},
        "O": {}
    }

    prev_num = 0
    i = 0
    try:
        for i, line in enumerate(lines):
            num, process, event = [x.strip() for x in line.split(":")]

            # cislovani radku
            try:
                num = int(num)
            except ValueError as e:
                raise ValueError(f"Line index is '{num}', expected a number")
            if (num != prev_num + 1):
                raise ValueError(f"Line index is '{num}', does not follow '{prev_num}' on the previous line")
            if (num != i+1):
                raise ValueError(f"Line index is '{num}', does not match the line number")

            prev_num = num
            process = process.split()
            event = event.split()

            # rozdeleni procesu
            match process:
                case ["P"]: # proces privoz
                    if ferry.state == FerryState.FINISHED:
                        raise ValueError("Ferry acted after finishing")

                    match event:
                        case ["started"]:
                            if ferry.state != FerryState.UNKNOWN:
                                raise ValueError("Ferry started twice")
                            # elif num != 1:
                            #     raise ValueError("Ferry didn't start on line 1")
                            ferry.state = FerryState.STARTED

                        case ["finish"]:
                            if ferry.state == FerryState.UNKNOWN or ferry.state == FerryState.STARTED:
                                raise ValueError("Ferry finished without arriving to a port")
                            elif ferry.state != FerryState.EN_ROUTE:
                                raise ValueError(f"Ferry finished without leaving port {ferry.port}")
                            # elif ferry.loaded["N"] + ferry.loaded["O"] > 0:
                            #     raise ValueError(f"Ferry finished with {ferry.loaded['N']} trucks and {ferry.loaded['O']} cars on board")
                            ferry.state = FerryState.FINISHED

                        case ["arrived", "to", port] if port.isdigit() and int(port) in range(2):
                            if ferry.port == "Dock" and port != "0":
                                raise ValueError(f"Ferry must arrive to port 0 after starting, not port {port}")
                            elif ferry.port == port:
                                raise ValueError(f"Ferry arrived to the same port it departed from (port {port})")
                            elif not (ferry.state == FerryState.STARTED or ferry.state == FerryState.EN_ROUTE):
                                raise ValueError(f"Ferry arrived to port {port} without leaving port {ferry.port}")
                            ferry.port = port
                            ferry.state = FerryState.ARRIVED_UNLOADING

                        case ["leaving", port] if port.isdigit() and int(port) in range(2):
                            if ferry.state == FerryState.EN_ROUTE and port == ferry.port:
                                raise ValueError(f"Ferry left port {port} twice")
                            elif not (ferry.state == FerryState.ARRIVED_UNLOADING or ferry.state == FerryState.ARRIVED_BOARDING):
                                raise ValueError(f"Ferry left port {port} without arriving at it first")
                            elif ferry.port != port:
                                raise ValueError(f"Ferry left the wrong port (expected port {ferry.port}, got port {port})")
                            elif ferry.state == FerryState.ARRIVED_UNLOADING and ferry.loaded["N"] + ferry.loaded["O"] > 0:
                                raise ValueError(f"Ferry left port while it still had {ferry.loaded['N']} trucks and {ferry.loaded['O']} cars to unload")
                            ferry.state = FerryState.EN_ROUTE
                            ferry.trips += 1

                        case _:
                            raise ValueError(f"Invalid event '{' '.join(event)}'")

                case ["N" | "O" as vt, id] if id.isdigit(): # procesy aut
                    is_truck = vt == "N"
                    vehicle: Vehicle = vehicles[vt].get(id, None)
                    if event != ["started"] and vehicle == None:
                        raise ValueError(f"{vt} {id} acted before starting")

                    match event:
                        case ["started"]:
                            if id in vehicles[vt]:
                                raise ValueError(f"{vt} {id} started twice")
                            if is_truck:
                                vehicles[vt][id] = Truck(id)
                                if N != -1 and Truck.count > N:
                                   raise ValueError(f"Too many trucks started (expected {N}, found {Truck.count})")
                            else:
                                vehicles[vt][id] = Car(id)
                                if O != -1 and Car.count > O:
                                    raise ValueError(f"Too many cars started (expected {O}, found {Car.count})")

                        case ["arrived", "to", port] if port.isdigit() and int(port) in range(2):
                            if vehicle.state != VehicleState.STARTED:
                                raise ValueError(f"{vt} {id} arrived when it shouldn't have")
                            vehicle.state = VehicleState.WAITING
                            vehicle.port = port

                        case ["boarding"]:
                            if vehicle.state != VehicleState.WAITING:
                                raise ValueError(f"{vt} {id} boarded while not waiting to board")
                            elif not (ferry.state == FerryState.ARRIVED_UNLOADING or ferry.state == FerryState.ARRIVED_BOARDING):
                                raise ValueError(f"{vt} {id} boarded while the ferry is not at a port")
                            elif ferry.port != vehicle.port:
                                raise ValueError(f"{vt} {id} boarded at port {vehicle.port} while the ferry is at port {ferry.port}")
                            elif ferry.state == FerryState.ARRIVED_UNLOADING and ferry.loaded["N"] + ferry.loaded["O"] > 0:
                                raise ValueError(f"{vt} {id} boarded while the ferry still had {ferry.loaded['N']} trucks and {ferry.loaded['O']} cars to unload")
                            vehicle.state = VehicleState.BOARDED
                            ferry.state = FerryState.ARRIVED_BOARDING
                            ferry.loaded[vt] += 1
                            ferry.measured_capacity = max(ferry.measured_capacity, ferry.loaded["N"]*3 + ferry.loaded["O"])
                            if K != -1 and ferry.measured_capacity > K: # pouze pokud byl spusten run
                                raise ValueError(f"Ferry capacity exceeded (expected {K}, measured {ferry.measured_capacity})")

                        case ["leaving", "in", port] if port.isdigit() and int(port) in range(2):
                            if vehicle.state != VehicleState.BOARDED:
                                raise ValueError(f"{vt} {id} left while not on board")
                            elif ferry.state == FerryState.ARRIVED_BOARDING:
                                raise ValueError(f"{vt} {id} left after another vehicle boarded (all vehicles must leave before any can board)")
                            elif port == vehicle.port:
                                raise ValueError(f"{vt} {id} left at the same port it started in (port {vehicle.port})")
                            elif port != ferry.port:
                                raise ValueError(f"{vt} {id} left at port {port} while the ferry is at port {ferry.port}")
                            vehicle.state = VehicleState.LEFT
                            ferry.loaded[vt] -= 1
                            if (ferry.loaded[vt] < 0):
                                raise ValueError(f"Ferry unloaded more vehicles than it loaded")

                case _: # neplatny proces
                    raise ValueError(f"Invalid process '{' '.join(process)}'")

        i += 1
        # overeni namereneho poctu aut a kapacity, pokud byl spusten run
        if N != -1 and O != -1:
            if Truck.count != N:
                raise ValueError(f"Too few trucks started (expected {N}, found {Truck.count})")
            elif Car.count != O:
                raise ValueError(f"Too few cars started (expected {O}, found {Car.count})")
            if K != -1 and N*3 + O > K and ferry.measured_capacity < K:
                print(f"{Fore.YELLOW}{Style.BRIGHT}Warning: {Style.NORMAL}Ferry didn't fully utilize its capacity (expected {K}, measured {ferry.measured_capacity}){Style.RESET_ALL}")

        # ukonceni vsech procesu
        if ferry.state != FerryState.FINISHED or num != len(lines):
            raise ValueError("Ferry didn't finish on the last line")
        for vt in ["N", "O"]:
            for vehicle in vehicles[vt].values():
                if vehicle.state != VehicleState.LEFT:
                    raise ValueError(f"{vt} {vehicle.id} didn't reach its destination")

            if (len_id := len(vehicles[vt])) != (max_id := max([int(id) for id in vehicles[vt].keys()])):
                raise ValueError(f"'{vt}' processes have a gap in their ids (found {len_id} processes, highest id is {max_id})")

    # zachycene chyby, ukonceni testu
    except ValueError as e:
        print(f"{Style.BRIGHT}{Fore.RED}Error on line {i+1}: {Style.NORMAL}{e}{Style.RESET_ALL}")
        sys.exit(1)

    finally:
        print(ferry)
        print(f"Trucks started: {Truck.count}\nCars started: {Car.count}")
    print(f"{Style.BRIGHT}{Fore.GREEN}No errors found{Style.RESET_ALL}")


if __name__ == "__main__":
    p = argparse.ArgumentParser(prog="test.py")
    s = p.add_subparsers(dest="cmd")

    a = s.add_parser("analyze")
    a.add_argument("log_file", nargs="?", default=DEFAULT_OUT, help=f"log file to analyze (default \"{DEFAULT_OUT}\")")
    a.add_argument("N", nargs="?", type=int, default=-1, help="optional N parameter to check against")
    a.add_argument("O", nargs="?", type=int, default=-1, help="optional O parameter to check against")
    a.add_argument("K", nargs="?", type=int, default=-1, help="optional K parameter to check against")

    r = s.add_parser("run")
    r.add_argument("path_to_binary", help="path to binary")
    for name in ("N", "O", "K", "TC", "TP"):
        r.add_argument(name, help=f"{name} parameter")
    r.add_argument("--log-file", dest="log_file", default=DEFAULT_OUT, help=f"path to log file to analyze after run (default \"{DEFAULT_OUT}\")")

    args = p.parse_args()
    if args.cmd == "analyze":
        analyze(args.log_file, args.N, args.O, args.K)
    elif args.cmd == "run":
        run(args.path_to_binary, args.N, args.O, args.K, args.TC, args.TP)
        analyze(args.log_file, int(args.N), int(args.O), int(args.K))
    else:
        p.print_help()
        sys.exit(1)