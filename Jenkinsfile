pipeline {
    agent { label 'DOCKEREXECUTER' }
    options {
       preserveStashes(buildCount: 5)
    }
    stages {
        stage('Build') {
           steps {
                echo 'Building..'
            	agent {
                	docker { image 'cicd_arduinocli:latest' }
            	}
		steps {
    			// some block
	      		sh label: 'arduino copy board', returnStatus: true, script: 'cp -r ~/Arduino .'
			sh label: 'arduino copy libs', returnStatus: true, script: 'cp -r ~/.arduino15 .'
			sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli -v compile --build-path ${WORKSPACE}/target/ --fqbn "esp8266:esp8266:d1_mini_pro" easyexample_master2'
	      		//stash the result
	      		stash includes: '**/*.bin', name: 'arduino'
		}
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
		sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli -v upload -i target/easyexample_master.ino.bin --fqbn "esp8266:esp8266:d1_mini_pro" -p /dev/ttyUSB0'
            }
        }
    }
}
