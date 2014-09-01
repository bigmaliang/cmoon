#!/usr/bin/expect -f

set user [lindex $argv 0]
set passwd [lindex $argv 1]
set copyip [lindex $argv 2]
#set port  [lindex $argv 3] 
set file1  [lindex $argv 3] 
set file2  [lindex $argv 4]

spawn  /usr/bin/scp $file1 $user@$copyip:$file2
expect {
   "password:" {
      send "${passwd}\r"
      }
   "yes/no)?" {
      send "yes\r"
      }
 }    
expect "root@"
