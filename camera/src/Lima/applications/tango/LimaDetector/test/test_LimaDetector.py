from PyTango import *
import unittest
import sys
import time
import random

class TestLimaDetectorDevice(unittest.TestCase):

	limadetector_device_name = "flo/op/motor.h"
	print "==============================================="
	limadetector = DeviceProxy(limadetector_device_name)
	print "-> ping limadetector(\"",limadetector_device_name,"\") ",limadetector.ping()," us"
	
	
	def setUp(self):
		
		
		self.seq = range(10)

	def test_snap(self):
        # make sure the shuffled sequence does not lose any elements
		limadetector.snap()
		random.shuffle(self.seq)
		self.seq.sort()
		self.assertEqual(self.seq, range(10))

        # should raise an exception for an immutable sequence
		self.assertRaises(TypeError, random.shuffle, (1,2,3))

	def test_choice(self):
		element = random.choice(self.seq)
		self.assertTrue(element in self.seq)

	def test_sample(self):
		with self.assertRaises(ValueError):
			random.sample(self.seq, 20)
		for element in random.sample(self.seq, 5):
			self.assertTrue(element in self.seq)

if __name__ == '__main__':
	unittest.main()

