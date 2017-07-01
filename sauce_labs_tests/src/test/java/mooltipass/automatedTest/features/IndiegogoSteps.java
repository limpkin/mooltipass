package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Indiegogo;

public class IndiegogoSteps {
	Indiegogo ind= new Indiegogo(WebDriverFactory.get());

	@When("I login Indiegogo with '(.*)'")
	public void login(String username){
		ind.goToLogin();
		ind.enterEmail(username);
		String password=System.getenv().get("INDIEPASS");
		ind.enterPassword(password);
		ind.submit();
		
	}
	@When("I go to Indiegogo login page")
	public void pressLogin(){
		ind.goToLogin();
		Assert.assertTrue("Expected to be at login page", ind.checkAtLoginPage());
		
	}
	@Then("I should be logged in Indiegogo")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",ind.checkLogin());
	}
	
	@When("I logout Indiegogo")
	public void pressLogout(){
		ind.goTodDashboard();
		ind.logout();
	}
}
