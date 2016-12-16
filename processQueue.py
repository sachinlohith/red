import time, os, signal, sys

if len(sys.argv) < 2:
    print "Usage: python clearQueue.py timeout"
    exit(1)

with open('pid', 'r') as fp:
    pid = int(fp.read())

timeout = float(sys.argv[1])

# Clear the queue of server.c after timeout seconds
while(True):
    time.sleep(timeout)
    os.kill(pid, signal.SIGTSTP)
