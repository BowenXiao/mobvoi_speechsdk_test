#! /usr/bin/env python
# -*- coding: utf-8 -*-  
import sys, os, commands
# import json
import re
import csv

WORKSPACE = sys.path[0]
RESULT_FILE = os.path.join(WORKSPACE,'result_offline')
RES_DIR = "./res/"
AUDIO_FILE_FORMAT = '.wav'

def getAudioNames():
	'''
	获取所有音频文件名
	return list
	'''
	p = re.compile('\./res/(.*)\.wav')
	names = commands.getoutput("cat %s | awk  '{print $1}'"%RESULT_FILE)
	audio_name_list = re.findall(p, names)
	return audio_name_list

def getIntent():
	'''
	获取所有Intent结果
	return list
	'''
	p = re.compile('wav (.*)\n')
	Intent_list = commands.getoutput("cat %s"%RESULT_FILE)
	Intent_list = re.findall(p, Intent_list)
	# print Intent_list[0]
	return Intent_list

def dget(dictionary, cmd, default=None):
	'''
	获取嵌套字典中的value
	'''
	cmd_list = cmd.split('.')
	tmp = dict(dictionary)
	for c in cmd_list:
		try:
			val = tmp.get(c, None)
		except AttributeError:
			return default
		if val!= None:
			tmp = val
		else:
			return default
	return tmp

def buildmResult():
	'''
	生成测试结果字典集
	return list
	'''
	intents = [eval(intent) for intent in getIntent()]
	audios = getAudioNames()
	mRet=[]
	for index in range(len(intents)):
		result={}
		intent = intents[index]
		#　返回error code时，将其转换成字典统一处理
		if isinstance(intents[index],int):
			intent = {}
		result['name'] = audios[index]
		result.update(intent)
		# print result
		mRet.append(result)
	return mRet

def buildRaw():
	'''
	生成CSV文件并将结果按照固定格式写入
	'''
	datas = buildmResult()
	# print datas
	with open('Result.csv','w') as f:
		writer = csv.writer(f)
		for data in datas:
			if len(data) > 2:
				f.write("%s,%s,,,,,"%(dget(data,'name'),dget(data,'query')))
				if 'domain' in data.keys():
					f.write("%s=%s,"%('domain',dget(data,'domain')))
					# print ">>>>>>>>>> domain: " + dget(data,'domain')
				else:
					# print ">>>>>>>>>> domain: NULL"
					f.write("NULL,")
				if 'action' in dget(data,'clientAction'):
					f.write("%s=%s,"%('action',dget(data,'clientAction.action')))
					# print ">>>>>>>>>> action: " + dget(data,'clientAction.action')
				else:
					# print ">>>>>>>>>> action: NULL"
					f.write("NULL,")
				#将extras中所有key的值写入CSV
				if dget(data,'clientAction.extras'):
					for key in dget(data,'clientAction.extras').keys():
						f.write("%s=%s,"%(key,dget(data,'clientAction.extras.%s'%key)))
						# print ">>>>>>>>>> extras: " + dget(data,'clientAction.extras')
			# 将返回error code的数据按照特殊格式处理
			else:
				f.write("%s,NULL,,,,,"%(dget(data,'name')))
			f.write("\r")

if __name__ == '__main__':
	buildRaw()