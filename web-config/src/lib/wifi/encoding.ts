/**
 * Simple XOR-based password encoding using device MAC address as key
 * Provides basic obfuscation to prevent casual inspection
 */

export function encodePassword(password: string, mac: string): string {
  if (!password) return '';
  
  // Remove colons from MAC to get consistent key
  const key = mac.replace(/:/g, '');
  
  let encoded = '';
  for (let i = 0; i < password.length; i++) {
    const charCode = password.charCodeAt(i) ^ key.charCodeAt(i % key.length);
    encoded += String.fromCharCode(charCode);
  }
  
  // Base64 encode to make it URL-safe
  return btoa(encoded);
}

export function decodePassword(encoded: string, mac: string): string {
  if (!encoded) return '';
  
  try {
    // Base64 decode
    const decoded = atob(encoded);
    const key = mac.replace(/:/g, '');
    
    let password = '';
    for (let i = 0; i < decoded.length; i++) {
      const charCode = decoded.charCodeAt(i) ^ key.charCodeAt(i % key.length);
      password += String.fromCharCode(charCode);
    }
    
    return password;
  } catch (e) {
    console.error('Failed to decode password:', e);
    return '';
  }
}
