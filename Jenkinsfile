pipeline {
    agent any
    options {
        skipDefaultCheckout(true)
        timestamps()
        timeout(time: 30, unit: 'MINUTES')
    }
    stages {
        stage('Checkout') { steps { checkout scm } }
        stage('Configure') {
            steps { sh 'cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug' }
        }
        stage('Build') { steps { sh 'cmake --build build --parallel 2' } }
        stage('Unit and system tests') {
            steps {
                sh 'ctest --test-dir build --output-on-failure --output-junit build/test-results.xml'
                sh 'python3 scripts/validate.py --executable build/traffic_sim --output-dir build/system-reports'
            }
        }
        stage('Static analysis') {
            steps {
                sh 'python3 scripts/format.py'
                sh 'cmake -S . -B build-analysis -DBUILD_TESTING=OFF -DTRAFFIC_ENABLE_ANALYZER=ON'
                sh 'cmake --build build-analysis --parallel 2'
            }
        }
        stage('Coverage') {
            steps {
                sh 'python3 scripts/coverage.py --output-dir build/coverage'
                sh 'python3 scripts/check_traceability.py --junit build/coverage/tests.xml'
            }
        }
        stage('Container smoke test') {
            steps {
                sh 'docker build -t traffic-sim:jenkins-$BUILD_NUMBER .'
                sh 'docker run --rm traffic-sim:jenkins-$BUILD_NUMBER --scenario config/acceptance_scenario.txt'
            }
            post { always { sh 'docker image rm traffic-sim:jenkins-$BUILD_NUMBER || true' } }
        }
    }
    post {
        always {
            junit allowEmptyResults: true, testResults: 'build/test-results.xml,build/coverage/tests.xml'
            archiveArtifacts allowEmptyArchive: true,
                artifacts: 'build/test-results.xml,build/coverage/**,build/system-reports/**,build-analysis/analysis-*.plist'
        }
    }
}
