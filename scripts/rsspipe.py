#! /usr/local/bin/python
# 
# rsspipe.py v0
#   <http://rentzsch.com/code/rsspipe>
#   Copyright (c) 2004 Jonathan 'Wolf' Rentzsch.
#   Some rights reserved: <http://creativecommons.org/licenses/by/2.0/>
"""
Reads lines from stdin, outputting an RSS 0.92 file containing the last N lines as items.
usage example:
	tail -f /var/log/httpd/access_log|python rsspipe.py --title 'http traffic' httpd_access.xml
"""

import sys, cgi
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-t", "--title", dest="title", help="feed title", default="A Project Twilight MUD RSS Feed")
parser.add_option("-l", "--link", dest="link", help="feed link", default="http://www.brandonsplace.net/projecttwilight/")
parser.add_option("-d", "--description", dest="description", help="feed description", default="A Project Twilight MUD RSS Feed")
parser.add_option("-i", "--limit", dest="limit", help="number of lines to export as RSS items", default=100)
(options, args) = parser.parse_args()

outputFile = open(args[0],'w')
lineQueue = []
while 1:
	readline = sys.stdin.readline()
	if readline != '':
		if len(lineQueue) >= int(options.limit):
			del lineQueue[0]
		lineQueue.append(readline.rstrip())
		
		outputString = '<?xml version="1.0"?>\n<rss version="0.92">\n<channel>\n<title>' + options.title + '</title>\n<link>' + options.link + '</link>\n<description>' + options.description + '</description>\n';
		for theLine in lineQueue:
			encodedLine = cgi.escape(theLine)
#			outputString = outputString + '<item><title>' + encodedLine + '</title><link>' + encodedLine + '</link><description>' + encodedLine + '</description></item>';

			outputString = outputString + '<item><title>' + encodedLine + '</title>\n<link>http://www.brandonsplace.net/projecttwilight/</link>\n<description>' + encodedLine + '</description>\n</item>\n';
		outputString = outputString + '</channel>\n</rss>'
		outputFile.seek(0)
		outputFile.truncate(0)
		outputFile.write(outputString)
		outputFile.flush()
	if readline == '':
		break
