pipeline {
    agent any

    stages {
        stage('Build') {
	   agent { label 'ardoinocli' }
           steps {
              echo 'Building..'
	      sh label: 'arduino', returnStatus: true, script: '/usr/local/arduino-cli/arduino-cli -v compile --fqbn "esp8266:esp8266:d1_mini_pro" easyexample_master'
	      //stash the result
	      stash includes: '/home/jenkins-slave/.arduino15/*', name: 'arduino'
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
            }
        }
    }
}
