#ifndef WINHTTP_ERROR_C
#define WINHTTP_ERROR_C

char*
winhttp_getstr(DWORD dw)
{
  switch (dw)
  {
    case ERROR_WINHTTP_CANNOT_CONNECT:
      return "Returned if connection to the server failed.";

    case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED:
        " The secure HTTP server requires a client certificate. The application retrieves the list of certificate issuers by calling WinHttpQueryOption with the WINHTTP_OPTION_CLIENT_CERT_ISSUER_LIST option.\n"
          "If the server requests the client certificate, but does not require it, the application can alternately call WinHttpSetOption with the WINHTTP_OPTION_CLIENT_CERT_CONTEXT option. In this case, the application specifies the WINHTTP_NO_CLIENT_CERT_CONTEXT macro in the lpBuffer parameter of WinHttpSetOption. For more information, see the WINHTTP_OPTION_CLIENT_CERT_CONTEXT option.Windows Server 2003 with SP1, Windows XP with SP2 and Windows 2000:  This error is not supported.";

    case ERROR_WINHTTP_CONNECTION_ERROR:
        return "The connection with the server has been reset or terminated, or an incompatible SSL protocol was encountered. For example, WinHTTP version 5.1 does not support SSL2 unless the client specifically enables it.";

    case ERROR_WINHTTP_INCORRECT_HANDLE_STATE:
        return "The requested operation cannot be carried out because the handle supplied is not in the correct state.";

    case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE:
        return "The type of handle supplied is incorrect for this operation.";

    case ERROR_WINHTTP_INTERNAL_ERROR:
        return "An internal error has occurred.";

    case ERROR_WINHTTP_INVALID_URL:
        return "The URL is invalid.";

    case ERROR_WINHTTP_LOGIN_FAILURE:
        return "The login attempt failed. When this error is encountered, the request handle should be closed with WinHttpCloseHandle. A new request handle must be created before retrying the function that originally produced this error.";

    case ERROR_WINHTTP_NAME_NOT_RESOLVED:
        return "The server name cannot be resolved.";

    case ERROR_WINHTTP_OPERATION_CANCELLED:
        return "The operation was canceled, usually because the handle on which the request was operating was closed before the operation completed.";

    case ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW:
        return "Returned when an incoming response exceeds an internal WinHTTP size limit.";

    case ERROR_WINHTTP_SECURE_FAILURE:
        return "One or more errors were found in the Secure Sockets Layer (SSL) certificate sent by the server. To determine what type of error was encountered, verify through a WINHTTP_CALLBACK_STATUS_SECURE_FAILURE notification in a status callback function. For more information, see WINHTTP_STATUS_CALLBACK.";

    case ERROR_WINHTTP_SHUTDOWN:
        return "The WinHTTP function support is shut down or unloaded.";

    case ERROR_WINHTTP_TIMEOUT:
        return "The request timed out.";

    case ERROR_WINHTTP_UNRECOGNIZED_SCHEME:
        return "The URL specified a scheme other than \"http:\" or \"https:\".";

    case ERROR_NOT_ENOUGH_MEMORY:

        return "Not enough memory was available to complete the requested operation. (Windows error code)\n"
          "Windows Server 2003, Windows XP and Windows 2000:  The TCP reservation range set with the WINHTTP_OPTION_PORT_RESERVATION option is not large enough to send this request.";

    case ERROR_INVALID_PARAMETER:

        return "The content length specified in the dwTotalLength parameter does not match the length specified in the Content-Length header.\n"
          "The lpOptional parameter must be NULL and the dwOptionalLength parameter must be zero when the Transfer-Encoding header is present.\n"
          "The Content-Length header cannot be present when the Transfer-Encoding header is present.";

    case ERROR_WINHTTP_RESEND_REQUEST:
        return "The application must call WinHttpSendRequest again due to a redirect or authentication challenge\n."
          "Windows Server 2003 with SP1, Windows XP with SP2 and Windows 2000:  This error is not supported.";
    default: break;
  }
  return "Unknown";
}
#endif //WINHTTP_ERROR_C
