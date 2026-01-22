
#include <Windows.h>
#include <aclapi.h>

namespace {
	class SecurityAttributes {
	public:
		SecurityAttributes() {
			Initialize();
		}

		~SecurityAttributes() {
			Release();
		}

		const SECURITY_ATTRIBUTES& operator*() const { 
			return sa;
		}

	private:
		PSID pEveryoneSID = nullptr;
		PSID pAdminSID = nullptr;
		PACL pACL = nullptr;
		PSECURITY_DESCRIPTOR pSD = nullptr;
		SECURITY_ATTRIBUTES sa;

		bool Initialize() {
			DWORD dwRes;

			EXPLICIT_ACCESS ea[2];
			SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
			SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;

			// Create a well-known SID for the Everyone group.
			if (!AllocateAndInitializeSid(&SIDAuthWorld, 1,
					SECURITY_WORLD_RID,
					0, 0, 0, 0, 0, 0, 0,
					&pEveryoneSID)) {
				Release();
				return false;
			}

			// Initialize an EXPLICIT_ACCESS structure for an ACE.
			// The ACE will allow Everyone read access to the key.
			ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));
			ea[0].grfAccessPermissions = KEY_ALL_ACCESS;
			ea[0].grfAccessMode = SET_ACCESS;
			ea[0].grfInheritance = NO_INHERITANCE;
			ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
			//ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
			ea[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
			ea[0].Trustee.ptstrName = (LPTSTR)pEveryoneSID;

			// Create a SID for the BUILTIN\Administrators group.
			if (!AllocateAndInitializeSid(&SIDAuthNT, 2,
					SECURITY_BUILTIN_DOMAIN_RID,
					DOMAIN_ALIAS_RID_ADMINS,
					0, 0, 0, 0, 0, 0,
					&pAdminSID)) {
				Release();
				return false;
			}

			// Initialize an EXPLICIT_ACCESS structure for an ACE.
			// The ACE will allow the Administrators group full access to
			// the key.
			ea[1].grfAccessPermissions = KEY_ALL_ACCESS;
			ea[1].grfAccessMode = SET_ACCESS;
			ea[1].grfInheritance = NO_INHERITANCE;
			ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
			ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
			ea[1].Trustee.ptstrName = (LPTSTR)pAdminSID;

			// Create a new ACL that contains the new ACEs.
			dwRes = SetEntriesInAcl(2, ea, NULL, &pACL);
			if (ERROR_SUCCESS != dwRes) {
				Release();
				return false;
			}

			// Initialize a security descriptor.  
			pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
			if (NULL == pSD) {
				Release();
				return false;
			}

			if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
				Release();
				return false;
			}

			// Add the ACL to the security descriptor. 
			if (!SetSecurityDescriptorDacl(pSD,
					true,     // bDaclPresent flag   
					pACL,
					false)) {  // not a default DACL 
				Release();
				return false;
			}

			// Initialize a security attributes structure.
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.lpSecurityDescriptor = pSD;
			sa.bInheritHandle = false;
			return true;
		}

		void Release() {
			if (pEveryoneSID)
				FreeSid(pEveryoneSID);
			if (pAdminSID)
				FreeSid(pAdminSID);
			if (pACL)
				LocalFree(pACL);
			if (pSD)
				LocalFree(pSD);
		}
	};
} // namespace

const SECURITY_ATTRIBUTES& getSecurityAttributes() {
	static SecurityAttributes s;
	return *s;
}
