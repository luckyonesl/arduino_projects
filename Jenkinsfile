pipeline {
    agent any

    stages {
        stage('Build') {
	   //agent { label 'ardoinocli' }
           steps {
              echo 'Building..'
              //sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli core install esp8266:esp8266'
              //sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli lib install PubSubClient RTCVars Ticker "BlueDot BME280 Library"'
	      	sh label: 'arduino copy board', returnStatus: true, script: 'cp -r ~/Arduino .'
		sh label: 'arduino copy libs', returnStatus: true, script: 'cp -r ~/.arduino15 .'
		sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli -v compile --fqbn "esp8266:esp8266:d1_mini_pro" easyexample_master'
	      //stash the result
	      //stash includes: '/home/jenkins-slave/.arduino15/*', name: 'arduino'
	      stash includes: '**/*', name: 'arduino'
           }
        }
        stage('Test') {
            steps {
                echo 'Testing..'
            }
        }
        stage('Flash') {
	    agent { label 'ARM' }
            steps {
                echo 'Flash....'
		//unstash 'arduino'
		//sh label: 'arduino new sketch', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli sketch new HelloWorld'
		//sh label: 'arduino compile', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli -v compile --fqbn "esp8266:esp8266:d1_mini_pro" -p /dev/ttyUSB0 HelloWorld'
		//sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli -v upload --fqbn "esp8266:esp8266:d1_mini_pro" -p /dev/ttyUSB0 HelloWorld'
            }
        }
    }
}
