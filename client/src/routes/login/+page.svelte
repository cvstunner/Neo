<script lang="ts">
  import { page } from "$app/stores";

  const errMap: Record<string, string> = {
    not_allowed: "This Google account is not authorized to access NEO.",
    oauth_failed: "Google sign-in failed. Please try again.",
  };

  $: error = $page.url.searchParams.get("error");
</script>

<svelte:head><title>Sign In — NEO</title></svelte:head>

<div class="login-page">
  <div class="glow-1"></div>
  <div class="glow-2"></div>

  <div class="card">
    <div class="brand">
      <div class="brand-mark">N</div>
      <span class="brand-name">NEO</span>
    </div>

    <h1 class="heading">Welcome back</h1>
    <p class="sub">Sign in to access your personal assistant dashboard.</p>

    {#if error}
      <div class="error-box">
        <svg
          width="15"
          height="15"
          fill="none"
          stroke="currentColor"
          stroke-width="2"
          viewBox="0 0 24 24"
          ><circle cx="12" cy="12" r="10" /><line
            x1="12"
            y1="8"
            x2="12"
            y2="12"
          /><line x1="12" y1="16" x2="12.01" y2="16" /></svg
        >
        {errMap[error] ?? "An error occurred. Please try again."}
      </div>
    {/if}

    <a href="http://localhost:3000/api/auth/google" class="google-btn">
      <svg width="18" height="18" viewBox="0 0 24 24">
        <path
          fill="#4285F4"
          d="M22.56 12.25c0-.78-.07-1.53-.2-2.25H12v4.26h5.92c-.26 1.37-1.04 2.53-2.21 3.31v2.77h3.57c2.08-1.92 3.28-4.74 3.28-8.09z"
        />
        <path
          fill="#34A853"
          d="M12 23c2.97 0 5.46-.98 7.28-2.66l-3.57-2.77c-.98.66-2.23 1.06-3.71 1.06-2.86 0-5.29-1.93-6.16-4.53H2.18v2.84C3.99 20.53 7.7 23 12 23z"
        />
        <path
          fill="#FBBC05"
          d="M5.84 14.09c-.22-.66-.35-1.36-.35-2.09s.13-1.43.35-2.09V7.07H2.18C1.43 8.55 1 10.22 1 12s.43 3.45 1.18 4.93l2.85-2.22.81-.62z"
        />
        <path
          fill="#EA4335"
          d="M12 5.38c1.62 0 3.06.56 4.21 1.64l3.15-3.15C17.45 2.09 14.97 1 12 1 7.7 1 3.99 3.47 2.18 7.07l3.66 2.84c.87-2.6 3.3-4.53 6.16-4.53z"
        />
      </svg>
      Continue with Google
    </a>

    <p class="footnote">Access is restricted to authorized accounts only.</p>
  </div>
</div>

<style>
  .login-page {
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
    background: var(--color-bg-base);
    position: relative;
    overflow: hidden;
  }

  .glow-1 {
    position: absolute;
    width: 520px;
    height: 520px;
    pointer-events: none;
    background: radial-gradient(
      circle,
      rgba(124, 106, 247, 0.1) 0%,
      transparent 70%
    );
    top: -120px;
    right: -120px;
  }
  .glow-2 {
    position: absolute;
    width: 400px;
    height: 400px;
    pointer-events: none;
    background: radial-gradient(
      circle,
      rgba(16, 185, 129, 0.06) 0%,
      transparent 70%
    );
    bottom: -80px;
    left: -80px;
  }

  .card {
    background: #13131c;
    border: 1px solid #1e1e2e;
    border-radius: 20px;
    padding: 48px 44px;
    width: 100%;
    max-width: 420px;
    position: relative;
    z-index: 1;
    box-shadow: 0 32px 80px rgba(0, 0, 0, 0.6);
    animation: fadeIn 0.45s ease both;
  }

  @keyframes fadeIn {
    from {
      opacity: 0;
      transform: translateY(18px);
    }
    to {
      opacity: 1;
      transform: translateY(0);
    }
  }

  .brand {
    display: flex;
    align-items: center;
    gap: 10px;
    margin-bottom: 32px;
  }
  .brand-mark {
    width: 38px;
    height: 38px;
    border-radius: 11px;
    background: linear-gradient(135deg, #7c6af7, #a78bfa);
    display: flex;
    align-items: center;
    justify-content: center;
    font-family: var(--font-display);
    font-weight: 800;
    font-size: 18px;
    color: #fff;
    box-shadow: 0 6px 20px rgba(124, 106, 247, 0.4);
  }
  .brand-name {
    font-family: var(--font-display);
    font-weight: 700;
    font-size: 20px;
    letter-spacing: 4px;
    color: #f1f0fb;
  }

  .heading {
    font-family: var(--font-display);
    font-size: 26px;
    font-weight: 700;
    color: #f1f0fb;
    margin-bottom: 8px;
  }
  .sub {
    font-size: 14px;
    color: #7a7690;
    margin-bottom: 28px;
    line-height: 1.5;
  }

  .error-box {
    display: flex;
    align-items: center;
    gap: 8px;
    background: rgba(244, 63, 94, 0.1);
    border: 1px solid rgba(244, 63, 94, 0.2);
    color: #f43f5e;
    padding: 12px 16px;
    border-radius: 10px;
    font-size: 13px;
    margin-bottom: 20px;
  }

  .google-btn {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 12px;
    width: 100%;
    padding: 13px 20px;
    background: #fff;
    color: #1a1a2e;
    border-radius: 12px;
    text-decoration: none;
    font-size: 15px;
    font-weight: 600;
    transition: all 0.2s;
    box-shadow: 0 2px 12px rgba(0, 0, 0, 0.3);
    font-family: var(--font-sans);
  }
  .google-btn:hover {
    background: #f0f0f0;
    transform: translateY(-1px);
    box-shadow: 0 6px 20px rgba(0, 0, 0, 0.4);
  }

  .footnote {
    text-align: center;
    font-size: 12px;
    color: #4a4760;
    margin-top: 20px;
  }
</style>
