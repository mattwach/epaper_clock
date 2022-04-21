#!/bin/env python3

import sys

def calc_checksum(line):
  if not line.startswith('$'):
    print(f'{line} -> FAIL (does not start with $)')
    return

  data = line[1:]

  if data.count('*') != 1:
    print(f'{line} -> FAIL (* count should be 1)')
    return

  data, reported_checksum = data.split('*')
  reported_checksum = reported_checksum[:2]
  checksum = 0
  for c in data:
    checksum = checksum ^ ord(c)

  checksum = '%02X' % checksum
  if checksum == reported_checksum:
    print(f'{line} -> OK')
  else:
    print(f'{line} -> FAIL expected: {checksum}')



def main():
  if len(sys.argv) != 1:
    sys.exit('usage cat <messages> | calc_checksum')
  for line in sys.stdin:
    calc_checksum(line.strip())

if __name__ == '__main__':
  main()
