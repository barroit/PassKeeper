/****************************************************************************
**
** Copyright 2023, 2024 Jiamu Sun
** Contact: barroit@linux.com
**
** This file is part of PassKeeper.
**
** PassKeeper is free software: you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation, either version 3 of the License, or (at your
** option) any later version.
**
** PassKeeper is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with PassKeeper. If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

HANDLE xCreateFile(
	LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	HANDLE out;

	out = CreateFile(lpFileName, dwDesiredAccess, dwShareMode,
			  lpSecurityAttributes, dwCreationDisposition,
			   dwFlagsAndAttributes, hTemplateFile);

	if (out == INVALID_HANDLE_VALUE)
	{
		BOOL is_creation =
			dwCreationDisposition == CREATE_ALWAYS ||
			 dwCreationDisposition == CREATE_NEW;

		die_winerr("unable to %s %s", is_creation ? "create" : "open", lpFileName);
	}

	return out;
}

WINBOOL xDuplicateHandle(
	HANDLE hSourceProcessHandle, HANDLE hSourceHandle,
	HANDLE hTargetProcessHandle, LPHANDLE lpTargetHandle,
	DWORD dwDesiredAccess, WINBOOL bInheritHandle, DWORD dwOptions)
{
	DWORD errnum;

	if (!DuplicateHandle(
		hSourceProcessHandle, hSourceHandle,
		 hTargetProcessHandle, lpTargetHandle,
		  dwDesiredAccess, bInheritHandle, dwOptions))
	{
		errnum = GetLastError();
		CloseHandle(hSourceHandle);

		SetLastError(errnum);
		die_winerr("failed to duplicate handle ‘%p’ to ‘%p’",
				*lpTargetHandle, hSourceHandle);
	}

	return TRUE;
}

WINBOOL xSetStdHandle(DWORD nStdHandle, HANDLE hHandle)
{
	DWORD errnum;

	if (!SetStdHandle(nStdHandle, hHandle))
	{
		errnum = GetLastError();
		CloseHandle(hHandle);

		SetLastError(errnum);
		die_winerr("failed to set standard handle ‘%d’ to ‘%p’",
				nStdHandle, hHandle);
	}

	return TRUE;
}
