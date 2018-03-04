package config

import (
	"testing"
)

func TestConfParsing(t *testing.T) {
	t.Log("Test conf file parsing")

	var cf *conf_t
	var err error
	var text = `a=b#comment
	b=1
	c
	d=#comment2
	[section]
	e f = 12
	g=on
	a='on'`

	throw := func() { t.Error("Error parsing:", text) }

	cf, err = ParseConf(text)
	if err != nil {
		throw()
	}

	if cf.GetString("default", "a", "") != "b" {
		throw()
	}

	if cf.GetString("default", "c", "non-empty") != "" {
		throw()
	}

	if cf.GetString("default", "d", "non-empty") != "" {
		throw()
	}

	if cf.GetInt("section", "ef", 0) != 12 {
		throw()
	}

	if cf.GetBool("section", "g", false) != true {
		throw()
	}

	if cf.GetString("section", "a", "") != "on" {
		throw()
	}

	text = `a="ab\"sd"
	b='single-quote\n\tnew-line'
	c="#=@#!$#*("

	d=1
	d=2
	d=3
	d=4
	d=5`

	cf, err = ParseConf(text)
	if err != nil {
		throw()
	}

	if cf.GetString("default", "a", "") != `ab"sd` {
		throw()
	}

	if cf.GetString("default", "b", "") != "single-quote\n\tnew-line" {
		throw()
	}

	if cf.GetString("default", "c", "") != "#=@#!$#*(" {
		throw()
	}

	for i, num := range cf.GetArray("default", "d") {
		if n, ok := num.(float64); ok {
			if int(n) == i+1 {
				continue
			}
		}

		throw()
	}

	text = `[sect#ion
	a=b`

	cf, err = ParseConf(text)
	if err != nil && err.(*ConfError).text == "[" {
		// ok
	} else {
		throw()
	}

	text = `a= ='#v'`

	cf, err = ParseConf(text)
	if err != nil && err.(*ConfError).text == "=" {
		// ok
	} else {
		throw()
	}

	text = `a= 'text\'`

	cf, err = ParseConf(text)
	if err != nil && err.(*ConfError).text == "'" {
		// ok
	} else {
		throw()
	}

	text = `a= "incomplete #comment`

	cf, err = ParseConf(text)
	if err != nil && err.(*ConfError).text == "\"" {
		// ok
	} else {
		throw()
	}
}
