#!/usr/bin/groovy

@Library(['github.com/indigo-dc/jenkins-pipeline-library@1.0.0']) _

pipeline {
    agent {
        label 'go'
    }
    
    environment {
        dockerhub_repo = "indigodatacloud/oidc-agent"
        dockerhub_image_id = ""
    }

    stages {
        stage('Code fetching') {
            steps {
                checkout scm
            }
        }
           
        stage('Metrics gathering') {
            agent {
                label 'sloc'
            }
            steps {
                dir("$WORKSPACE/oidc-agent") {
                    SLOCRun()
                }
            }
            post {
                success {
                    dir("$WORKSPACE/oidc-agent") {
                        SLOCPublish()
                    }
                }
            }
        }
        stage('Dependency check') {
            agent {
                label 'docker-build'
            }
            steps {
                OWASPDependencyCheckRun("$WORKSPACE/oidc-agent", project="oidc-agent")
            }
            post {
                always {
                    OWASPDependencyCheckPublish()
                    HTMLReport(
                        "$WORKSPACE/oidc-agent",
                        'dependency-check-report.html',
                        'OWASP Dependency Report')
                    deleteDir()
                }
            }
        }

        stage('Style Analysis') {
            steps {
                sh '''
                GOFMT="/usr/local/go/bin/gofmt -s"
                bad_files=$(find . -name "*.go" | xargs $GOFMT -l)
                if [[ -n "${bad_files}" ]]; then
                    echo "!!! '$GOFMT' needs to be run on the following files: "
                    echo "${bad_files}"
                    exit 1
                fi
                '''            
            }
            
        }


    
        /*stage('DockerHub delivery') {
            when {
                anyOf {
                    branch 'master'
                    buildingTag()
                }
		    }
            agent {
                label 'docker-build'
            }
            steps {
                sh 'printenv'
                //TOBEADDED

                }
            }
            post {
                
                success {
                    echo "Pushing Docker image ${dockerhub_image_id}.."
                    DockerPush(dockerhub_image_id)
                }
                failure {
                    echo 'Docker image building failed, removing dangling images..'
                    DockerClean()
                }
                always {
                    cleanWs()
                }
            }
        } */

         stage('Build RPM/DEB packages') {
            /*when {
                anyOf {
                    buildingTag()
                    branch 'master'
                }
            }*/
            parallel {
                stage('Build on Ubuntu16.04') {
                    agent {
                        label 'bubuntu16'
                    }
                    steps {
                        checkout scm
                        sh ''' 
                            echo 'Within build on Ubuntu16.04'
                            echo 'Installing build-dependencies'
                            sudo apt-get update && sudo apt-get install -y wget libcurl4-openssl-dev help2man libseccomp-dev libmicrohttpd-dev software-properties-common check
                            sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 8B48AD6246925553
                            sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 7638D0442B90D010
                            sudo add-apt-repository "deb http://ftp.debian.org/debian stretch-backports main"
                            sudo apt update && sudo apt install -t stretch-backports libsodium23 libsodium-dev
                            sudo make deb
                            mkdir -p UBUNTU
                            PARENT="$(dirname "$WORKSPACE")"
                            echo $PARENT
                            find "$PARENT" -type f -name "*oidc*.deb" -exec mv -v -t UBUNTU \'{}\' \';\'
                        '''            
                    }
                    post {
                        success {
                            archiveArtifacts artifacts: 'UBUNTU/*.deb'                        }
                    }
                } 
                stage('Build on CentOS7') {
                    agent {
                        label 'bcentos7'
                    }
                    steps {
                        checkout scm
                        sh '''
                            echo 'Within build on CentOS7'
                            sudo make rpm
                            mkdir -p RPMS
                            find .. -type f -name "oidc-agent*.rpm" -exec cp -v -t RPMS \'{}\' \';\'
                        '''
                    }
                    post {
                        success {
                            archiveArtifacts artifacts: 'RPMS/*.rpm'
                        }
                    }
                }
            }
        }

    
    }
}
