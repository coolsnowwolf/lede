#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# 功能：
#   通过NetworkManager刷新IP地址激活默认的网络连接
# 存在的问题：
#   1、许多64位Linux操作系统不支持DBus和NetworkManager，这种情况下本脚本将无法正常运行
#   2、由于njit-client程序尚不支持无线网络，故此处已经跳过所有的无线网络配置
#
# 当前代码由刘群<liuqun68@gmail.com>负责维护
# 欢迎您帮助我们改进这个Python脚本，来信请寄：
#   njit8201xclient@googlegroups.com
#   http://groups.google.com/group/njit8021xclient?hl=zh-CN


import sys
import dbus

try:
	nm = dbus.SystemBus().get_object('org.freedesktop.NetworkManager',
                                        '/org/freedesktop/NetworkManager')
except dbus.exceptions.DBusException, errmsg:
	sys.stderr.write(__file__ + ': Unable to find NetworkManager daemon from DBus: ' + errmsg)
	sys.exit(0xFF)

def GetProperty(property_name, object, interface_name):
	"""Get property of a dbus object through a dbus interface"""
	return dbus.Interface(object, 'org.freedesktop.DBus.Properties').Get(interface_name, property_name)

active_connections = GetProperty('ActiveConnections', nm, 'org.freedesktop.NetworkManager')
if active_connections == []:
	sys.stderr.write(__file__ + ': Unable to find any NetworkManager configurations!')
	sys.exit(0xFF)

class ActiveConnectionInfo:
	pass

for connection_opath in active_connections:
	ac = ActiveConnectionInfo()
	ac.obj = dbus.SystemBus().get_object('org.freedesktop.NetworkManager', connection_opath)
	ac.service    = GetProperty('ServiceName', ac.obj, 'org.freedesktop.NetworkManager.Connection.Active')
	ac.connection = GetProperty('Connection', ac.obj, 'org.freedesktop.NetworkManager.Connection.Active')
	ac.devices    = GetProperty('Devices', ac.obj, 'org.freedesktop.NetworkManager.Connection.Active')
	ac.spec       = GetProperty('SpecificObject', ac.obj, 'org.freedesktop.NetworkManager.Connection.Active')
	nm_setting_service = dbus.SystemBus().get_object(ac.service, ac.connection)
	ac.settings = dbus.Interface(nm_setting_service, 'org.freedesktop.NetworkManagerSettings.Connection').GetSettings()

	if ac.settings['connection']['type'] == '802-3-ethernet': # 只找出有线以太网连接连接（因njit-client尚不支持无线）
		for dev in ac.devices:
			print __file__ + ': Activating network connection and refreshing IP address...'
			print __file__ + ': Using NetworkManager configuration "' + ac.settings['connection']['id'] + '".'
			try:
				dbus.Interface(nm,'org.freedesktop.NetworkManager').ActivateConnection(ac.service, ac.connection, dev, ac.spec)
				sys.exit(0) # 正常退出
			except dbus.exceptions.DBusException, errmsg:
				sys.stderr.write(__file__ + ': Failed to activate "' + ac.settings['connection']['id'] + '": ' + errmsg)

sys.stderr.write(__file__ + ': Unable to find any wired network connections. Check your cables maybe?') 

