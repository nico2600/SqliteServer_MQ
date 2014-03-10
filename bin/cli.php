#!/usr/bin/php
<?php

$host = ($_SERVER['argc'] == 2) ? $_SERVER['argv'][1] : 'localhost';
$port = 11139;
if (($sock = fsockopen($host, $port)) === false) {
	fprintf(STDOUT, "Unable to connect to the server.\n");
	exit(1);
}
while (true) {
	print('> ');
	$input = input();
	$cmd = substr($input, 0, 5);
	$cmd = trim($cmd);
	$input = substr($input, 6);
	$input = trim($input);
	if (!strcasecmp($cmd, 'help'))
		command_help();
	else if (!strcasecmp($cmd, 'query'))
		command_query($sock, $input);
	else if (!strcasecmp($cmd, 'quit'))
		exit();
	print("\n");
}

/**
 * Process a GET command.
 * @param	$sock	stream	The connection socket.
 * @param	$input	string	The command string.
 */
function command_query($sock, $input) {
	// extract key and value
	if ($input[0] != '"' || ($pos = strpos($input, '"', 1)) === false) {
		print("\tSyntax error\n");
		return;
	}
   print "input:$input #$pos\n";
   $query = substr($input, 1, $pos-1);
   print "query: $query\n";
	// send the command
	$buffer = chr(1) . chr(mb_strlen($query, 'ascii')+1) . $query.chr(0);
   print("sending $buffer#\n");
	fwrite($sock, $buffer);
	$response = fread($sock, 1);
	if (ord($response) != 1) {
		print("\tERROR\n");
		return;
	}
	$response = fread($sock, 4);
	$length = unpack('Nint', $response);
	$length = $length['int'];
	$value = null;
	if ($length > 0) {
		$value = fread($sock, $length);
		print("$value\n");
	}
}

/** Show the help. */
function command_help() {
	print("\tquery \"cmp\" \n");
	print("\tquit\n");
}
/**
 * Get user input.
 * @return	The user input.
 */
function input() {
	$stdin = fopen('php://stdin', 'r');
	$s = trim(fgets($stdin));
	return ($s);
}
