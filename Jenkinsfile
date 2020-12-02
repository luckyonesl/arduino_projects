pipeline {
    agent { label 'DOCKEREXECUTER' }
    options {
       preserveStashes(buildCount: 5)
    }
    stages {
      stage('Build') {
            agent {
               docker { 
                  image 'cicd_arduinocli:latest'
                  args "--entrypoint=''"
               }
            }
            steps {
               sh label: 'arduino copy board', returnStatus: true, script: 'cp -r /home/jenkins-slave/Arduino .'
               sh label: 'arduino copy libs', returnStatus: true, script: 'cp -r  /home/jenkins-slave/.arduino15 .'
               sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli -v compile --build-path ${WORKSPACE}/target/ --fqbn "esp8266:esp8266:d1_mini_pro" fsexample'
               stash includes: '**/*.bin', name: 'arduino'
            }
      }
      stage('FLASH'){
         agent { label 'ARM' }
            steps {
               echo 'Flash....'
		         unstash 'arduino'
               sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli core install esp8266:esp8266'
		         sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli -v upload -i target/fsexample.ino.bin --fqbn "esp8266:esp8266:d1_mini_pro" -p /dev/ttyUSB0'
            }
      }
    }
}



