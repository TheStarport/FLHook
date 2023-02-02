#include "Global.hpp"
#include <openssl/pem.h>
#include <openssl/x509.h>

/* Generates a self-signed x509 certificate. */
X509* GenerateX509(EVP_PKEY* pkey)
{
	/* Allocate memory for the X509 structure. */
	X509* x509 = X509_new();
	if (!x509)
	{
		Console::ConErr(L"Unable to create X509 structure.");
		return nullptr;
	}

	/* Set the serial number. */
	ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);

	/* This certificate is valid from now until exactly one year from now. */
	X509_gmtime_adj(X509_get_notBefore(x509), 0);
	X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);

	/* Set the public key for our certificate. */
	X509_set_pubkey(x509, pkey);

	/* We want to copy the subject name to the issuer name. */
	auto* name = X509_get_subject_name(x509);

	/* Set the country code and common name. */
	X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, reinterpret_cast<unsigned char*>("CA"), -1, -1, 0);
	X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, reinterpret_cast<unsigned char*>("FLServer"), -1, -1, 0);
	X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<unsigned char*>("localhost"), -1, -1, 0);

	/* Now set the issuer name. */
	X509_set_issuer_name(x509, name);

	/* Actually sign the certificate with our key. */
	if (!X509_sign(x509, pkey, EVP_sha1()))
	{
		Console::ConErr(L"Error signing certificate.");
		X509_free(x509);
		return nullptr;
	}

	return x509;
}

bool WriteToDisk(EVP_PKEY* pkey, X509* x509)
{
	/* Open the PEM file for writing the key to disk. */
	FILE* pkey_file = fopen("key.pem", "wb");
	if (!pkey_file)
	{
		Console::ConErr(L"Unable to open \"key.pem\" for writing.");
		return false;
	}

	/* Write the key to disk. */
	bool ret = PEM_write_PrivateKey(pkey_file, pkey, nullptr, nullptr, 0, nullptr, nullptr);
	fclose(pkey_file);

	if (!ret)
	{
		Console::ConErr(L"Unable to write private key to disk.");
		return false;
	}

	/* Open the PEM file for writing the certificate to disk. */
	FILE* x509_file = fopen("cert.pem", "wb");
	if (!x509_file)
	{
		Console::ConErr(L"Unable to open \"cert.pem\" for writing.");
		return false;
	}

	/* Write the certificate to disk. */
	ret = PEM_write_X509(x509_file, x509);
	fclose(x509_file);

	if (!ret)
	{
		Console::ConErr(L"Unable to write certificate to disk.");
		return false;
	}

	return true;
}

void GenerateCertificate()
{
	Console::ConInfo(L"Generating Certificate and PEM");
	Console::ConInfo(L"Generating RSA key");
	EVP_PKEY* key = EVP_RSA_gen(2048);
	if (!key)
	{
		Console::ConWarn(L"Unable to generate 2048-bit RSA key");
		return;	
	}

	/* Generate the certificate. */
	Console::ConInfo(L"Generating X509 certificate");

	X509* x509 = GenerateX509(key);
	if (!x509)
	{
		Console::ConWarn(L"Unable to generate X509 cert");
		EVP_PKEY_free(key);
		return;
	}

	const bool ret = WriteToDisk(key, x509);
	EVP_PKEY_free(key);
	X509_free(x509);

	if (!ret)
	{
		Console::ConWarn(L"Unable to write certificate and key to disk.");
	}
	else
	{
		Console::ConInfo(L"Certificate and key written to disk.");
	}
}