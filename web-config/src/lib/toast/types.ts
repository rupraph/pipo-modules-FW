export type Toast = {
  type: "info" | "success" | "warning" | "error";
  message: string;
  dismissible?: boolean;
  timeout: number;
};
export type StoreToast = Toast & { id: number };
