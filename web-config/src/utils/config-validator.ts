import type { PipoConfig, PipoTypes } from "../types";

/**
 * Validation result with detailed error information
 */
export interface ValidationResult {
  valid: boolean;
  errors: string[];
}

/**
 * Deep validation of config to find null/undefined values
 * Returns detailed error messages for all invalid fields
 */
export function validateConfigForSave<T extends PipoTypes>(
  config: PipoConfig<T>
): ValidationResult {
  const errors: string[] = [];

  if (!config) {
    return {
      valid: false,
      errors: ["Configuration is null or undefined"],
    };
  }

  // Helper function to check for null/undefined values recursively
  function checkValue(
    value: any,
    path: string,
    parentKey?: string
  ): void {
    if (value === null) {
      errors.push(`${path} is null`);
      return;
    }

    if (value === undefined) {
      errors.push(`${path} is undefined`);
      return;
    }

    // For objects, recursively check all properties
    if (typeof value === "object" && !Array.isArray(value)) {
      for (const [key, val] of Object.entries(value)) {
        checkValue(val, `${path}.${key}`, key);
      }
    }

    // For arrays, check each element (like current_scale)
    if (Array.isArray(value)) {
      value.forEach((item, index) => {
        checkValue(item, `${path}[${index}]`);
      });
    }

    // Check for empty strings in critical fields
    if (typeof value === "string" && value.trim() === "") {
      // Only flag empty strings for address fields and names
      if (
        parentKey === "osc_addr" ||
        parentKey === "PipoName" ||
        parentKey === "scaleType" ||
        parentKey === "pattern"
      ) {
        errors.push(`${path} is an empty string`);
      }
    }

    // Check for NaN values
    if (typeof value === "number" && isNaN(value)) {
      errors.push(`${path} is NaN`);
    }
  }

  // Validate general config
  if (config.general) {
    checkValue(config.general, "general");
  } else {
    errors.push("general config is missing");
  }

  // Validate inputs
  if (config.inputs) {
    checkValue(config.inputs, "inputs");
  } else {
    errors.push("inputs config is missing");
  }

  // Validate engine configs
  if (config.engine) {
    if (config.engine["engine-midi"]) {
      checkValue(config.engine["engine-midi"], "engine.engine-midi");
    } else {
      errors.push("engine.engine-midi is missing");
    }

    if (config.engine["engine-osc"]) {
      checkValue(config.engine["engine-osc"], "engine.engine-osc");
    } else {
      errors.push("engine.engine-osc is missing");
    }

    // engine-special is optional
    if (config.engine["engine-special"]) {
      checkValue(config.engine["engine-special"], "engine.engine-special");
    }
  } else {
    errors.push("engine config is missing");
  }

  // Validate sensor config
  if (config.sensorconf) {
    checkValue(config.sensorconf, "sensorconf");
  } else {
    errors.push("sensorconf is missing");
  }

  return {
    valid: errors.length === 0,
    errors,
  };
}

/**
 * Get a user-friendly summary of validation errors
 */
export function getValidationErrorSummary(result: ValidationResult): string {
  if (result.valid) {
    return "";
  }

  const errorCount = result.errors.length;
  const maxErrorsToShow = 5;

  let message = `Configuration has ${errorCount} invalid field${errorCount > 1 ? "s" : ""}:\n`;

  // Show first few errors
  const errorsToShow = result.errors.slice(0, maxErrorsToShow);
  message += errorsToShow.map((err) => `• ${err}`).join("\n");

  if (errorCount > maxErrorsToShow) {
    message += `\n... and ${errorCount - maxErrorsToShow} more issue${errorCount - maxErrorsToShow > 1 ? "s" : ""}`;
  }

  return message;
}
