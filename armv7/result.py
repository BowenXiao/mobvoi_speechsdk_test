#! /usr/bin/env python
# -*- coding: utf-8 -*-  
import sys, os, commands
# import json
import re
# import xlwt, csv

WORKSPACE = sys.path[0]
RESULT_FILE = os.path.join(WORKSPACE,'result')
RES_DIR = "./res/"
AUDIO_FILE_FORMAT = '.wav'

def getAudioNames():
	names = commands.getoutput("cat %s | awk  '{print $1}'"%RESULT_FILE)
	p = re.compile('\./res/(.*)\.wav')
	audio_name_list = re.findall(p, names)
	return audio_name_list

def getNLU():
	nlu_list = commands.getoutput("cat %s | awk  '{print $2}'"%RESULT_FILE).split('\n')
	return nlu_list

def buildResult():
	nlu_list = [eval(nlu) for nlu in getNLU()]
	audio_name_list = getAudioNames()
	mNLU = []
	update_key = ['clientAction','extras']
	for nlu in nlu_list:
		index = nlu_list.index(nlu)
		nlu['name'] = audio_name_list[index]
		#　将n dimension数据扁平处理为one dimension.
		for key in update_key:
			nlu.update(nlu[key])
			nlu.pop(key)
		# 将无效的GARBAGE数据归一化为一个Vector,仅保存query的值
		if nlu['query'] == 'GARBAGE':
			nlu.pop('domain')
			nlu.pop('action')
		#　异常数据处理
		elif not isinstance(nlu,dict):
			nlu['query'] = 'error'
		mNLU.append(nlu)
	# print mNLU
	return mNLU

def buildRaw():
	datas = buildResult()
	print datas
	# print datas
	with open('Result.csv','w') as f:
		writer = csv.writer(f)
		for data in datas:
			# data = json.dumps(data,encoding="UTF-8",ensure_ascii=False)
			# 将结果以固定顺序写入csv保证结果的可读性
			f.write("%s,%s,,,,,Intent,"%(data.pop('name'),data.pop('query')))
			if 'domain' in data.keys():
				f.write("%s=%s,"%('domain',data.pop('domain')))
			if 'action' in data.keys():
				f.write("%s=%s,"%('action',data.pop('action')))
			for key in data.keys():
				f.write("%s=%s,"%(key,data[key]))
			f.write("\r")

if __name__ == '__main__':
	buildRaw()