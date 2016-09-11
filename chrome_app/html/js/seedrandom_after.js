// These commands has to be executed right after seedrandom.js is loaded
// It's in an own file because inline-scripting is not allowed in a Chrome app
Math.seedrandom(initial_random_value, { entropy: true });