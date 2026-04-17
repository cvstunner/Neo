# PROJECT SETUP 
node -v
npm -v
psql --version
brew --version

# Install pnpm (faster than npm, we'll use this)
npm install -g pnpm

# Verify
pnpm -v


brew install mosquitto

# Start it as a background service
brew services start mosquitto

# Verify it's listening on port 1883
nc -z localhost 1883 && echo "✓ MQTT broker running" || echo "✗ Not running"



# DB SETUP
psql -U chetan -d postgres -c "CREATE DATABASE neo_db;"

\c neo_db

OR directly going to DB >> psql -U chetan -d neo_db

\dt