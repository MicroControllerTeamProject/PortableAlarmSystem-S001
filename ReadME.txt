- when set sms in text mode: 

mySim900->ATCommand("AT+CPMS=\"SM\"");
delay(500);
mySim900->ATCommand("AT+CMGF=1");
delay(500);
mySim900->ATCommand("AT+CMGD=1,4");
delay(1000);

use delay time after commands,becouse there are situations when sms don't comes.