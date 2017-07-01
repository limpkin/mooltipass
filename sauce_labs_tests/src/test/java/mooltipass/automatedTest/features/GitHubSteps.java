package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.GitHub;

public class GitHubSteps {
	GitHub github= new GitHub(WebDriverFactory.get());

	@When("I login GitHub with '(.*)'")
	public void login(String username){
		github.goToLogin();
		github.enterEmail(username);
		String password =System.getenv().get("GITPASS");
		github.enterPassword(password);
		github.submit();
		
	}
	@When("I go to GitHub login page")
	public void pressLogin(){
		github.goToLogin();
		Assert.assertTrue("Expected to be at login page", github.checkAtLoginPage());
		
	}
	@Then("I should be logged in GitHub")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",github.checkLogin());
	}
	
	@When("I logout GitHub")
	public void pressLogout(){
		github.goTodDashboard();
		github.logout();
	}
}
