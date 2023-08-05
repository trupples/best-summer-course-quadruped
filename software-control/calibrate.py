from math import pi
import requests
import itertools

# REPLACE THIS WITH YOUR ROBOT's IP ADDRESS
BASE = "http://192.168.1.119"

print("Welcome to the robot calibration tool!")

joints = [''.join(parts) for parts in itertools.product('fb', 'lr', '-', 'hk')]
print(joints)

def state(modifications={}):
	print("STATE", modifications)
	states = requests.get(f"{BASE}/state?" + '&'.join(f"{k}={v}" for k, v in modifications.items())).text.strip().split("\n")
	states = [line.strip().split(" ") for line in states]
	states = {line[1]: (float(line[2]) if '.' in line[2] else int(line[2])) for line in states}
	return states

def config(modifications={}):
	configs = requests.get(f"{BASE}/config?" + '&'.join(f"{k}={v}" for k, v in modifications.items())).text.strip().split("\n")
	configs = [line.strip().split(" ") for line in configs]
	configs = {line[1]: (float(line[2]) if '.' in line[2] else int(line[2])) for line in configs}
	return configs

def calibrate_quantity(name, c, typefn=int):
	while True:
		print("Current value:", c[name])
		x = input("New calibration value (empty input = all good): ").strip()
		if x == "":
			break
		c = config({name: typefn(x)})
	return c

while True:
	print("")
	print("Options:")
	print("1. Move all legs to the mounting position")
	print("2. Calibrate one leg")
	print("*. Exit")
	opt = input()
	if opt == "1":
		print("!!!!!!! PLEASE BE CAREFUL !!!!!!!")
		print("If this is the first time building the robot, make sure you DETACH THE LIMBS before continuing with this step.")
		print("Only after running this may you mount the limbs.")
		if input("Are the limbs detached? (write yes) ") != "yes":
			print("Limbs are not detached, this operation is unsafe and will likely break your servos. Will not run.")
			continue

		state({"state": 0, **{ joint: pi/2 for joint in joints }})

	elif opt == "2":
		leg = input("Calibrate which leg? (fl, fr, bl, br)")
		if leg not in ["fl", "fr", "bl", "br"]:
			print("Invalid leg")
			continue

		input(f"Positioning leg {leg} straight down (press enter)")
		state({"state": 0, f"{leg}-h": pi/2, f"{leg}-k": 0})

		c = config()
		print("=== Make the leg straight (possibly not vertical)! ===")
		c = calibrate_quantity(leg + '-k-c0', c)
		print("=== Make the leg vertical! ===")
		c = calibrate_quantity(leg + '-h-c90', c)

		input(f"Positioning leg {leg} backwards and down (press enter)")
		state({"state": 0, f"{leg}-h": 0, f"{leg}-k": pi/2})

		print("=== Make the femur horizontal! ===")
		c = calibrate_quantity(leg + '-h-c0', c)
		print("=== Make the tibia vertical! ===")
		c = calibrate_quantity(leg + '-k-c90', c)

		print(f"Leg {leg} successfully calibrated!")
		print(f"Calibration values: {c[leg + '-h-c0']=} {c[leg + '-h-c90']=} {c[leg + '-k-c0']=} {c[leg + '-k-c90']=}")
	else:
		break
