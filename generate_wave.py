import math

num_samples = 800
frequency = 880
sample_rate = 8000

beep = []

for i in range(num_samples):
    t = i / sample_rate
    # signed 24 bit sample
    sample = int(0.5 * (2**23 - 1) * math.sin(2 * math.pi * frequency * t))
    beep.append(sample)

print("{", end="")
newline = 0
for sample in beep:
    print(f"{sample}, ", end="")
    newline += 1
    if newline == 10:
        print()
        newline = 0
print("}");