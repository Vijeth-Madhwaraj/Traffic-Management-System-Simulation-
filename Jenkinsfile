pipeline {
    agent any
    stages {
        stage('Configure') { steps { sh 'cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug' } }
        stage('Build') { steps { sh 'cmake --build build --parallel' } }
        // TODO: add unit/system test, analysis, coverage, Docker, and archive stages.
    }
}
