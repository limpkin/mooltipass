package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class StackOverflow extends AbstractPage{
	
	public StackOverflow(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(id = "email")
	private WebElement email;

	@FindBy(id = "password")
	private WebElement password;

	@FindBy(id = "submit-button")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//a[@class='login-link btn-clear']")
	private WebElement loginBtn;
	
	@FindBy(xpath = "//a[text()='log out']")
	private WebElement logoutBtn;

	@FindBy(xpath = "//input[@value='Log Out']")
	private WebElement submitLogoutBtn;
	
	@FindBy(xpath = "//a[contains(@title,'A list of all')]")
	private WebElement expandBtn;


	public void enterEmail(String value){
		email.sendKeys(value);
	}
	
	public void enterPassword(String value){
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		loginBtn.click();
	}
	
	public void submit(){
		waitUntilAppears(submitLogin);
		submitLogin.click();
	}
	
	public boolean checkLogin(){
		waitUntilAppears(By.xpath("//div[@title='your reputation: 1']"));
		return isElementPresent(By.xpath("//div[@title='your reputation: 1']"));
	}
	
	public void logout(){
		expandBtn.click();
		logoutBtn.click();
		submitLogoutBtn.click();
	}
	
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("email"));
	}
}
