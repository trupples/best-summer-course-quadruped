import signal
import sys
from time import time, sleep
from math import *
import requests

BASE = "http://192.168.1.165"

def state(modifications={}):
	url = f"{BASE}/state?" + '&'.join(f"{k}={v}" for k, v in modifications.items())
	print(url)
	states = requests.get(url).text.strip().split("\n")
	states = [line.strip().split(" ") for line in states]
	states = {line[1]: (float(line[2]) if '.' in line[2] else int(line[2])) for line in states}
	return states


H = -16.0
G = 3.0
xoff = 0


"""
for i in range(10):
	state({
		"fl-tx": 3,
		"fl-ty": H,
		"fr-tx": -3,
		"fr-ty": H,
		"bl-tx": 3,
		"bl-ty": H,
		"br-tx": -3,
		"br-ty": H
	})

	sleep(1)

	state({
		"fl-tx": -3,
		"fl-ty": H,
		"fr-tx": 3,
		"fr-ty": H,
		"bl-tx": -3,
		"bl-ty": H,
		"br-tx": 3,
		"br-ty": H
	})


	sleep(1)
"""
"""
for i in range(10):
	
	state({
		"fl-tx": 0,
		"fl-ty": -10,

		"fr-tx": 0,
		"fr-ty": -10,

		"bl-tx": 0,
		"bl-ty": -5,

		"br-tx": 0,
		"br-ty": -5
	})
	sleep(1)
	
	state({
		"fl-tx": 0,
		"fl-ty": -18,

		"fr-tx": 0,
		"fr-ty": -18,

		"bl-tx": 0,
		"bl-ty": -5,

		"br-tx": 0,
		"br-ty": -5
	})
	sleep(1)



papapap()
"""
def idle(t):
	return {
		"fl": (0, H),
		"fr": (0, H),
		"bl": (0, H),
		"br": (0, H)
	}

def updown(t, leg):
	gait = idle(t)
	gait[leg] = (0, H + G - G * cos(2 * pi * t))
	return gait

def trot_stationary(t):
	if t < 0.5:
		return {
			"fl": (0, H + G - G * cos(4 * pi * t)),
			"fr": (0, H),
			"br": (0, H + G - G * cos(4 * pi * t)),
			"bl": (0, H)
		}
	else:
		return {
			"fl": (0, H),
			"fr": (0, H + G - G * cos(4 * pi * t)),
			"br": (0, H),
			"bl": (0, H + G - G * cos(4 * pi * t))
		}

def lift(x):
	return max(0, 1-2*abs(x-0.5))

def walk1(t):
	tt = t * 4
	step = int(tt)
	x = tt - step
	length = 8

	leg_order = ["fl", "br", "fr", "bl"]

	gait = {
		"fl": [-t*length, H],
		"fr": [-t*length, H],
		"bl": [-t*length, H],
		"br": [-t*length, H]
	}

	leg = leg_order[step]

	for i in range(step):
		# Previously moved legs
		gait[leg_order[i]][0] += length # = [(1-t)*length, H]

	# Lean forwards/backwards to shift CG
	#xoff = +1 if leg_order[step][0] == 'f' else -1
	#xoff = xoff * sin(pi*tt) * 3

	#xoff = 0

	# Apply lean
	#for leg in leg_order:
	#	gait[leg][0] += xoff

	# Currently moving leg
	x = tt - step
	gait[leg][0] += x * length
	gait[leg][1] += G * lift(x)

	# Tilt towards opposite leg
	for l in gait:
		dist = sum(a != b for a, b in zip(leg, l))
		# dist = 0 for the given leg
		# dist = 1 for adjacent legs
		# dist = 2 for the opposite leg
		gait[l][1] += (dist-1) * G/2 * 0

	print(f"{t:.02}: {gait['fl'][0]:.03},{gait['fl'][1]:.03}\t{gait['br'][0]:.03},{gait['br'][1]:.03}\t{gait['fr'][0]:.03},{gait['fr'][1]:.03}\t{gait['bl'][0]:.03},{gait['bl'][1]:.03})")

	return gait

def shifttest(t):
	tt = t * 6
	x = tt - int(tt)

	G = 2
	h = H
	if int(tt) == 0:
		return {
			"fl": [0, h],
			"fr": [0, h+x*G/2],
			"bl": [0, h+x*G/2],
			"br": [0, h+x*G]
		}
	if int(tt) >= 1 and int(tt) <= 4:
		angle = (tt-1) / 4 * 2 * pi * 2
		return {
			"fl": [G * sin(angle), h + G - G * cos(angle)],
			"fr": [0, h+G/2],
			"bl": [0, h+G/2],
			"br": [0, h+G]
		}
	if int(tt) == 5:
		return {
			"fl": [0, h],
			"fr": [0, h+(1-x)*G/2],
			"bl": [0, h+(1-x)*G/2],
			"br": [0, h+(1-x)*G]
		}


def animate(gaitfn, T):
	print(f"animate({gaitfn.__name__}, {T})")
	time = 0
	t0 = time
	while True:
		t = (time - t0) / T
		if t >= 1:
			return
		gait = gaitfn(t)
		for leg in ["fl", "br", "fr", "bl"]:
			gait[leg] = (gait[leg][0] + xoff, gait[leg][1]) # Global x offset for balance ???
		states = {
			line.split(" ")[1] : (float(line.split(" ")[2]) if '.' in line.split(" ")[2] else int(line.split(" ")[2]))
			for line in
			requests.get(f"{BASE}/state?" +
			f"fl-tx={gait['fl'][0]}&fl-ty={gait['fl'][1]}&" +
			f"fr-tx={gait['fr'][0]}&fr-ty={gait['fr'][1]}&" +
			f"bl-tx={gait['bl'][0]}&bl-ty={gait['bl'][1]}&" +
			f"br-tx={gait['br'][0]}&br-ty={gait['br'][1]}").text.strip().split("\n")
		}
		#print(gait)
		#print(states)
		time += 0.1
		sleep(0.1)


requests.get(f"{BASE}/state?state=1") # MANUAL_IK mode

signal.signal(signal.SIGINT, lambda _1,_2: (print('EXITING, returning to idle position'), animate(idle, 0.1), sys.exit(0)))


"""
animate(walk1, 2)
animate(walk1, 2)
animate(walk1, 2)
animate(walk1, 2)

psspspsps()
"""
# H = -15
# animate(idle, 1)



for i in range(5):
	H = -15
	animate(idle, 1)
	H = -12
	animate(idle, 1)

animate(shifttest, 3)

for i in range(10):
	animate(walk1, 4)

"""

animate(idle, 0.5)

sys.exit(0)
"""

"""
BEAT = 0.333

G = 0.7
for tilt in [
	[H-G, H+G],
	[H, H],
	[H-G, H+G],
	[H, H],

	[H+G, H-G],
	[H, H],
	[H+G, H-G],
	[H, H]
]:
	print('BEAT')
	state({'fl-ty': tilt[0], 'bl-ty': tilt[0], 'fr-ty': tilt[1], 'br-ty': tilt[1]})
	sleep(0.333)

state({'fl-tx': -3, 'fr-tx': -3, 'bl-tx': -3, 'br-tx': -3})
sleep(0.666)
state({'fl-tx': +3, 'fr-tx': +3, 'bl-tx': +3, 'br-tx': +3})
sleep(0.666)

animate(walk1, 0.333 * 8)

animate(idle, 0.1)

#for leg in ["fl", "fr", "bl", "br"]:
#	for i in range(2):
#		animate(lambda t: updown(t, leg), 2)

#for i in range(10):
#	animate(walk1, 4)

#for i in range(5):
#	print(i)
#	animate(trot_stationary, 1.5)

#animate(idle, 1)
"""
