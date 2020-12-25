pipeline {
    agent any
    options {
       preserveStashes(buildCount: 5)
    }
    stages {
        stage('Build') {
	        agent { label 'arduinocli' }
            steps {
                echo 'Building..'
                //sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli core install esp8266:esp8266'
                //sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli lib install PubSubClient RTCVars Ticker "BlueDot BME280 Library"'
	      	    sh label: 'arduino link board', returnStatus: true, script: 'ln -s ~/Arduino'
		          sh label: 'arduino link libs', returnStatus: true, script: 'ln -s ~/.arduino15'
		          sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli -v compile --build-path ${WORKSPACE}/target/ --fqbn "esp8266:esp8266:generall" az-envy'
	            //stash the result
	            //stash includes: '/home/jenkins-slave/.arduino15/*', name: 'arduino'
	            stash includes: '**/*.bin', name: 'arduino'
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
		        unstash 'arduino'
              sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli core install esp8266:esp8266'
		        sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli -v upload -i target/az-envy.ino.bin --fqbn "esp8266:esp8266:generall" -p /dev/ttyUSB0'
            }
        }
    }
}
